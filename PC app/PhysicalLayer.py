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
        self.read = b""

        t = Timer(0.1, self.receiver)
        t.start()

    def pass_down(self, data: NetworkDataPoint) -> None:
        import parsers
        out = parsers.encode_uint16(data.address)
        out += data.value

        print("[TCP] sending data: ", out)
        string = b""
        for byte in out:
            string += parsers.encode_uint8((byte & 0xF0) >> 4)
            string += parsers.encode_uint8(byte & 0x0F)
        string += b"\xff"
        self.conn.send(string)

    def receiver(self):
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

        i = 0

        t = Timer(0.1, self.receiver)
        t.start()

        while i < len(bytes_array):
            self.read += parsers.encode_uint8((bytes_array[i] << 4) | bytes_array[i+1])
            # print(self.read)
            i += 2
            if bytes_array[i] == 0xFF:
                print("full frame:", self.read)
                i += 1
                address = parsers.parse_uint16(self.read[0:2])
                data = NetworkDataPoint(address, self.read[2:])
                self.read = b""
                if self.net:
                    self.net.pass_up(data)



class UART(PHYLayer):
    def __init__(self):
        super().__init__()

        import serial
        self.ser = serial.Serial("COM6", 115200)

        self.read = b""
        t = Timer(0.1, self.receiver)
        t.start()

    def pass_down(self, data: NetworkDataPoint) -> None:
        import parsers
        out = parsers.encode_uint16(data.address)
        out += data.value

        print("[TCP] sending data: ", out)
        string = b""
        for byte in out:
            string += parsers.encode_uint8((byte & 0xF0) >> 4)
            string += parsers.encode_uint8(byte & 0x0F)
        string += b"\xff"
        self.ser.write(string)

    def receiver(self):
        bytes_array = self.ser.read_until(b"\xff")

        print("[TCP] received bytes_array: ", bytes_array)

        import parsers

        i = 0

        while i < len(bytes_array):
            self.read += parsers.encode_uint8((bytes_array[i] << 4) | bytes_array[i+1])
            # print(self.read)
            i += 2
            if bytes_array[i] == 0xFF:
                print("full frame:", self.read)
                i += 1
                address = parsers.parse_uint16(self.read[0:2])
                data = NetworkDataPoint(address, self.read[2:])
                self.read = b""
                if self.net:
                    self.net.pass_up(data)

        t = Timer(0, self.receiver)
        t.start()