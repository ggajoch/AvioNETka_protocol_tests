from threading import Timer
import socket

from packet import *
from byteMainp import *
from parsers import *



class TCP:
    def __init__(self):
        self.net = None

        TCP_IP = '127.0.0.1'
        TCP_PORT = 7171
        BUFFER_SIZE = 20  # Normally 1024, but we want fast response

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((TCP_IP, TCP_PORT))
        s.listen(1)

        self.conn, addr = s.accept()

        t = Timer(1, self.receiver)
        t.start()

    def register_network(self, net):
        self.net = net

    def send_packet(self, p):
        data = []
        data.extend(setUint16(p.address))
        data.append(p.id)
        # data.extend(encode_float(p.data))
        data.extend(p.dataRaw)

        print "sending data! ", data
        self.conn.send("".join([chr(i) for i in data]))
        import time
        time.sleep(0.1)

    def receiver(self):
        while 1:
            string = self.conn.recv(1000)
            if not string: break
            data = [ord(i) for i in string]
            print "received data: ", data

            p = Packet()
            p.address = getUint16(data[0:2])
            p.id = data[2]
            p.dataRaw = data[3:]
            if self.net:
                self.net.got_packet(p)