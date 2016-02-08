from threading import Timer
import socket

from StackInterfaces import *
from byteMainp import *


class PHYLayer(PHYInterface):
    def __init__(self):
        self.net = None

    def register_upper_layer(self, net: NETInterface):
        self.net = net

    def passDown(self, data: PHYDataStruct) -> None:
        pass


class TCP(PHYLayer):
    def __init__(self):
        super().__init__()

        tcp_ip = '127.0.0.1'
        tcp_port = 7171

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((tcp_ip, tcp_port))
        s.listen(1)

        self.conn, addr = s.accept()

        t = Timer(1, self.receiver)
        t.start()

    def passDown(self, data: PHYDataStruct) -> None:
        bytes_array = []
        bytes_array.extend(setUint16(data.address))
        bytes_array.extend(data.data)

        import sys
        string = b''.join([i.to_bytes(1, sys.byteorder) for i in bytes_array])
        print("[TCP] sending data: ", string)
        self.conn.send(string)
        import time
        time.sleep(0.01)

    def receiver(self):
        while 1:
            try:
                bytes_array = self.conn.recv(1000)
            except ConnectionResetError:
                print("Connection closed.")
                import sys
                sys.exit(0)
            if not bytes_array:
                break
            print("[TCP] received bytes_array: ", bytes_array)

            import sys
            data = PHYDataStruct()
            data.address = int.from_bytes(bytes_array[0:2], sys.byteorder)
            data.data = bytes_array[2:]
            if self.net:
                self.net.passUp(data)
