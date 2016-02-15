from NetworkLayer import Network
from PhysicalLayer import TCP
import sim, time


phy = TCP()
net = Network(phy)
phy.register_upper_layer(net)



sim.sync()


time.sleep(5)
# i=0
# while True:
#     sim.set(1, i)
#     i += 1
    #time.sleep(3)

# x = Module(net, 1)
# x.pass_up(DataPoint(0, b'\x00'))
# x.pass_up(DataPoint(255, b'\x00\x01\x02\x03\x00\x02\x01'))
# print(x.descriptors[0])
# x.pass_up(DataPoint(0, b'\x01'))
