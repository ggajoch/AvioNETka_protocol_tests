from packet import *
import eventlet, threading
import time

def ACKNeeded(id):
    if id == 250:
        return False
    return True


class NetworkNode:
    def __init__(self, phy):
        self.phy = phy
        self.sim = None
        self.semACK = {}

    def register_sim(self, sim):
        self.sim = sim

    def got_packet(self, packet):
        print "received packet!!: ", packet
        if packet.id == 250:
            print "ACK!"
            try:
                self.semACK[packet.address].release()
            except:
                print "not waiting for ACK"
            return
        if ACKNeeded(packet.id):
            print "sending ACK"
            self.send_packet(makeACKPacket(packet.address))
        self.sim.parse_packet(packet)

    def send_packet(self, packet):
        t = threading.Timer(0, self.send_packet_thr, [packet])
        t.start()

    def send_packet_thr(self, packet):
        if ACKNeeded(packet.id):
            while self.semACK.has_key(packet.address):
                time.sleep(0.01)  # quick workaround for waiting for a free line

            self.semACK[packet.address] = eventlet.semaphore.Semaphore()
            self.semACK[packet.address].acquire(timeout=0)
            while True:
                self.phy.send_packet(packet)
                if self.semACK[packet.address].acquire(timeout=2):
                    print "GOT ACK"
                    break
                print "TIMEOUT!!!"
            self.semACK.pop(packet.address)
        else:
            self.phy.send_packet(packet)
