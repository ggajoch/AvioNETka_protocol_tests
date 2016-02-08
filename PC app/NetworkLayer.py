import eventlet, threading
import time

from StackInterfaces import *
from DataStructs import *


class NetworkDescriptor:
    def __init__(self):
        self.ack = False
        self.data = True

    @staticmethod
    def data_command(ack: bool):
        val = NetworkDescriptor()
        val.ack = ack
        val.data = True
        return val

    @staticmethod
    def system_command(ack: bool):
        val = NetworkDescriptor()
        val.ack = ack
        val.data = False
        return val

class NetworkDescriptors:
    def __init__(self):
        self.map = \
            {(43690, 1): NetworkDescriptor.data_command(True),
             (43690, 2): NetworkDescriptor.data_command(True)}
        self.systemCommands = \
            {250: NetworkDescriptor.system_command(False)}

    def data_command(self, command):
        return not self.system_command(command)

    def system_command(self, command):
        return command in self.systemCommands

    def set(self, address: int, command: int, value: NetworkDescriptor):
        self.map[(address, command)] = value

    def get(self, address: int, command: int) -> NetworkDescriptor:
        if self.system_command(command):
            return self.systemCommands[command]
        try:
            return self.map[(address, command)]
        except KeyError:
            return None


class NetworkLayer(NETInterface):
    def __init__(self):
        self.phy = None
        self.fsx = None
        self.semACK = {}
        self.descriptors = NetworkDescriptors()

    def register_lower_layer(self, phy: PHYInterface):
        self.phy = phy

    def register_upper_layer(self, fsx: FSXInterface):
        self.fsx = fsx

    def passUp(self, data: PHYDataStruct):
        print("[NET] passUp: ", data.data)
        packet = NETDataStruct()
        packet.command = data.data[0]
        packet.data = data.data[1:]

        descriptor = self.descriptors.get(data.address, packet.command)

        if packet.command == 250:
            print("[NET] Received ACK packet from ", data.address)
            try:
                self.semACK[data.address].release()
            except:
                print("[NET] not waiting for ACK")

        if descriptor.ack:
            print("[NET] sending ACK")
            x = PHYDataStruct()
            x.address = data.address
            x.data = [250]
            self.phy.passDown(x)

        if descriptor.data and self.fsx:
            self.fsx.passUp(packet)

    def passDown(self, data: NETDataStruct):
        descriptor = self.descriptors.get(data.address, data.command)

        frame = PHYDataStruct()
        frame.address = data.address
        frame.data = [data.command]
        frame.data.extend(data.data)

        if not descriptor.ack:
            self.phy.passDown(frame)
            return
        while data.address in self.semACK:
            import time
            time.sleep(0.01)  # quick workaround for waiting for a free line

        self.semACK[data.address] = eventlet.semaphore.Semaphore()
        self.semACK[data.address].acquire(timeout=0)
        for i in range(1, 3):
            self.phy.passDown(frame)
            print("Waiting for ACK from ", data.address)
            import time
            time.sleep(0.001)
            if self.semACK[data.address].acquire(timeout=3):
                print("GOT ACK")
                break
            print("TIMEOUT!!!")
        self.semACK.pop(data.address)

