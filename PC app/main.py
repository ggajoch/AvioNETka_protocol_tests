import phy, network
tcpphy = phy.TCP()
n = network.NetworkNode(tcpphy)
tcpphy.register_network(n)


