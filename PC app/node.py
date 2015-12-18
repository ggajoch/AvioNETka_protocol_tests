from byteMainp import *
from parsers import *
from packet import *


class SimNode:
    def __init__(self):
        self.modules = {}

    def parse_subscription(self, packet):
        addr = packet.address
        idd = packet.dataRaw[0]
        fsxId = getUint32(packet.dataRaw[1:5])
        ack = packet.dataRaw[5]
        rx = packet.dataRaw[6]

        print "Subscription! (add %d, id %d) -> fsx %d | (ack %d, rx %d)" % (addr, idd, fsxId, ack, rx)
        s = Subscription(addr, idd, fsxId, ack, rx, parse_float)

        if not self.modules.has_key(packet.address):
            # first packet from module, add module to list
            self.modules[packet.address] = Module(packet.address)

        self.modules[packet.address].subscriptions[idd] = s

    def parse_data(self, packet):
        sub = self.modules[packet.address].subscriptions[packet.id]
        packet.data = sub.parser(packet.dataRaw)
        print "got data! (%d %d) -> %d = %f" %(packet.address, packet.id, sub.fsxID, packet.data)

    def update_data(self):
        for module in self.modules:
            for sub in module.subscriptions:
                #val = get()
                import sim
                val = sim.get_value(sub.fsxID)
                if val != sub.prevData:
                    sub.prevData = val
                    p = Packet()
                    p.address = sub.address
                    p.id = sub.id
                    p.data = val
                    p.dataRaw = encode_float(p.data)
                    #send(p)


    def parse_packet(self, packet):
        if packet.id == 251:
            self.parse_subscription(packet)
        else:
            self.parse_data(packet)


class Module:
    def __init__(self, addr):
        self.address = addr
        self.subscriptions = {}  # id -> subscription


class Subscription:
    def __init__(self, addr, id, fsxID, ackNeeded, txEnabled, parser):
        self.address = addr
        self.id = id
        self.fsxID = fsxID
        self.ackNeeded = ackNeeded
        self.txEnabled = txEnabled
        self.parser = parser
        self.prevData = None

