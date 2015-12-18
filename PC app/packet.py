class Packet:
    def __init__(self):
        self.address = 0
        self.id = 0
        self.dataRaw = []
        self.data = 0

    def __str__(self):
        res = "packet: "
        res += "addr: %d, " % self.address
        res += "id: %d, " % self.id
        res += "data: " + str(self.dataRaw)
        return res


def makeACKPacket(address):
    p = Packet()
    p.id = 250
    p.address = address
    p.dataRaw = []
    return p