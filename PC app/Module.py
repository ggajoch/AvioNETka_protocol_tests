import threading
from SystemCommands import *


class Module:
    def __init__(self, net, address: int):
        self.address = address
        self.net = net

        self.ACK = ACK(self)
        self.SendSubscriptions = SendSubscriptions(self)
        self.semACK = threading.Lock()

        self.descriptors = {
            ACK.id: self.ACK,
            Ping.id: Ping(self),
            RegisterCommand.id: RegisterCommand(self),
            SendSubscriptions.id: self.SendSubscriptions
        }

    def subscribe(self):
        print("Sending subscribe packet")
        self.SendSubscriptions.send()

    def pass_up(self, data: DataPoint):
        print("Passing up id=", data.id)
        if data.id in self.descriptors:
            if self.descriptors[data.id].ack:
                print("Sending ACK (id = %d)" % data.id)
                self.ACK.send()
            self.descriptors[data.id].pass_up(data.value)
        else:
            print("Unregistered command! (addr = %d, id = %d)" % (self.address, data.id))

    def pass_down(self, data: DataPoint):
        print("Passing down data (id=", data.id, ", data= ", data.value, ", ack =", self.descriptors[data.id].ack, ")")
        from NetworkDataPoint import NetworkDataPoint

        out = NetworkDataPoint(self.address, encode_uint8(data.id) + data.value)
        if self.descriptors[data.id].ack:
            self.semACK.acquire(True)
            for i in range(1, 10):
                self.net.pass_down(out)
                print("Waiting for ACK from ", self.address)
                if self.semACK.acquire(True, 1):
                    self.semACK.release()
                    print("[NET] GOT ACK from ", self.address)
                    break

                print("[NET] TIMEOUT from ", self.address)
                self.net.modules.pop(self.address)
        else:
            self.net.pass_down(out)
