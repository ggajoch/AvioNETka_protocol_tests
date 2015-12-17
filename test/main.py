import socket
from threading import Timer
from time import sleep

def ackRequired(id):
    if id == 251 or id == 252:
        return True
    else:
        return False


def send(array):
    print "sending ", array
    conn.send("".join([chr(i) for i in array]))
    sleep(0.1)



TCP_IP = '127.0.0.1'
TCP_PORT = 7171
BUFFER_SIZE = 20  # Normally 1024, but we want fast response

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((TCP_IP, TCP_PORT))
s.listen(1)

conn, addr = s.accept()


def receiver():
    loss = 3
    while 1:
        string = conn.recv(BUFFER_SIZE)
        if not string: break
        data = [ord(i) for i in string]
        print "received data: ", data


        id = data[0]
        #if ackRequired(id):

        loss -= 1
        if loss == 0:
            loss = 3
        else:           
            arr = [0xfa]
            send(arr)

        #conn.send(data)  # echo
    print "closing connection"
    conn.close()


t = Timer(0.01, receiver)
t.start()

#sleep(4)
#send([252])

print 'Connection address:', addr
