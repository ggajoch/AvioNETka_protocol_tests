from parsers import parse, encode
import sim
from DataPoint import DataPoint


class DataCommand:
    def __init__(self, module, id_, encoding, fsxid, ack):
        self.module = module
        self.id = id_
        self.encoding = encoding
        self.fsxid = fsxid
        self.ack = ack
        sim.set_subscription(self.changed, self.fsxid)

    def changed(self, value):
        data = DataPoint(self.id, encode(self.encoding, value))
        self.module.pass_down(data)

    def pass_up(self, data):
        val = parse(self.encoding, data)
        print("Got data id=%d: " % self.id, val)
        sim.set(self.fsxid, float(val))

    def __str__(self):
        return "command ack=" + str(self.ack) + ", encoding=" + str(self.encoding) + ", FSXid=" + str(self.fsxid)
