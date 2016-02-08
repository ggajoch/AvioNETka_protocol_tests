import PhysicalLayer, NetworkLayer, DataStructs

tcpphy = PhysicalLayer.TCP()
net = NetworkLayer.NetworkLayer()

tcpphy.register_upper_layer(net)
net.register_lower_layer(tcpphy)

import time
time.sleep(5)
print("\n\nsimulating values!\n\n")
x = DataStructs.NETDataStruct()
x.address = 43690
x.command = 1
x.data = [2, 3, 4, 5]
net.passDown(x)
