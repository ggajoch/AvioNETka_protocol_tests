from threading import Timer
import socket
from NetworkDataPoint import *


class PHYLayer:
    def __init__(self):
        self.net = None

    def register_upper_layer(self, net):
        self.net = net

    def pass_down(self, data: NetworkDataPoint) -> None:
        pass


class TCP(PHYLayer):
    def __init__(self):
        super().__init__()

        #tcp_ip = '127.0.0.1'
        tcp_ip = '192.168.8.102'
        tcp_port = 7171

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((tcp_ip, tcp_port))
        s.listen(1)

        self.conn, addr = s.accept()

        t = Timer(0.1, self.receiver)
        t.start()

    def pass_down(self, data: NetworkDataPoint) -> None:
        import parsers
        out = parsers.encode_uint16(data.address)
        out += data.value

        print("[TCP] sending data: ", out)
        self.conn.send(out)
        import time
        time.sleep(0.01)

    def receiver(self):
        while 1:
            try:
                bytes_array = self.conn.recv(1024)
            except ConnectionResetError:
                print("Connection closed.")
                import sys
                sys.exit(0)
            if not bytes_array:
                print("[TCP] ERROR!")

            print("[TCP] received bytes_array: ", bytes_array)

            import parsers
            address = parsers.parse_uint16(bytes_array[0:2])
            data = NetworkDataPoint(address, bytes_array[2:])
            if self.net:
                self.net.pass_up(data)
