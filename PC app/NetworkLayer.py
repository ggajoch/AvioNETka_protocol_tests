from Module import Module
from NetworkDataPoint import *
from DataPoint import *


class Network:
    def __init__(self, phy):
        self.modules = {}
        self.phy = phy

    def pass_up(self, data: NetworkDataPoint):
        print("Received data from %d:" % data.address, data.value)
        new_mod = False
        if data.address not in self.modules:
            print("New module! address =", data.address)
            self.modules[data.address] = Module(self, data.address)
            new_mod = True


        idd = data.value[0]
        val = data.value[1:]

        self.modules[data.address].pass_up(DataPoint(idd, val))

        if new_mod:
            self.modules[data.address].subscribe()

    def pass_down(self, data: NetworkDataPoint):
        self.phy.pass_down(data)
