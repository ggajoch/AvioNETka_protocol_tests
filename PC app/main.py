import phy, network, node
tcpphy = phy.TCP()
net = network.NetworkNode(tcpphy)
tcpphy.register_network(net)

sim = node.SimNode(net)
net.register_sim(sim)

import time
time.sleep(5)
print "\n\nsimulating values!\n\n"
sim.update_data()
sim.update_data()
sim.update_data()