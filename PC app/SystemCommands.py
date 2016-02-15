from DataPoint import *
from parsers import *


class ACK:
    id = 250

    def __init__(self, module):
        self.ack = False
        self.module = module

    def pass_up(self, data: str):
        try:
            self.module.semACK.release()
        except:
            pass

    def send(self):
        data = DataPoint(self.id, b"")
        self.module.pass_down(data)


class RegisterCommand:
    id = 255

    def __init__(self, module):
        self.ack = True
        self.module = module

    def pass_up(self, data: str):
        from DataCommand import DataCommand

        idd = data[0]
        fsx_id = parse_uint32(data[1:5])
        encoder = data[5]
        ack = parse_bool(data[6:7])
        datacmd = DataCommand(self.module, idd, encoder, fsx_id, ack)
        print("Registering new data:", datacmd)
        self.module.descriptors[idd] = datacmd


class Ping:
    id = 251

    def __init__(self, module):
        self.ack = False
        self.module = module

    def pass_up(self, data: str):
        print("Got PING")
        if data[0] == 1:
            print("Responding PING")
            out = DataPoint(self.id, b"\x02")
            self.module.pass_down(out)


class SendSubscriptions:
    id = 252

    def __init__(self, module):
        self.ack = False
        self.module = module

    def send(self):
        data = DataPoint(self.id, b"")
        self.module.pass_down(data)

    def pass_up(self, data: str):
        pass

