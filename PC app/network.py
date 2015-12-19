from packet import *

class NetworkNode:
    def __init__(self, phy):
        self.phy = phy
        self.sim = None

    def register_sim(self, sim):
        self.sim = sim

    def got_packet(self, packet):
        print "received packet!: ", packet
        if packet.id != 250:
            self.send_packet(makeACKPacket(packet.address))
            self.sim.parse_packet(packet)


    def send_packet(self, packet):
        self.phy.send_packet(packet)
