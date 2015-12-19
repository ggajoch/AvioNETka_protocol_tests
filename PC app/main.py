import phy, network, node
tcpphy = phy.TCP()
net = network.NetworkNode(tcpphy)
tcpphy.register_network(net)

sim = node.SimNode(net)
net.register_sim(sim)
