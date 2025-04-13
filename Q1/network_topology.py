#!/usr/bin/python

from mininet.net import Mininet
from mininet.node import Controller
from mininet.node import Host
from mininet.node import OVSKernelSwitch
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import TCLink

def myNetwork():
    net = Mininet(topo=None, build=False, ipBase='10.0.0.0/24')

    info('*** Adding controller\n')
    c0 = net.addController(name='c0')

    info('*** Add switches\n')
    s1 = net.addSwitch('s1', cls=OVSKernelSwitch)
    s2 = net.addSwitch('s2', cls=OVSKernelSwitch)
    s3 = net.addSwitch('s3', cls=OVSKernelSwitch)
    s4 = net.addSwitch('s4', cls=OVSKernelSwitch)

    info('*** Add hosts\n')
    h1 = net.addHost('h1', cls=Host, ip='10.0.0.2/24')
    h2 = net.addHost('h2', cls=Host, ip='10.0.0.3/24')
    h3 = net.addHost('h3', cls=Host, ip='10.0.0.4/24')
    h4 = net.addHost('h4', cls=Host, ip='10.0.0.5/24')
    h5 = net.addHost('h5', cls=Host, ip='10.0.0.6/24')
    h6 = net.addHost('h6', cls=Host, ip='10.0.0.7/24')
    h7 = net.addHost('h7', cls=Host, ip='10.0.0.8/24')
    h8 = net.addHost('h8', cls=Host, ip='10.0.0.9/24')

    info('*** Add links\n')
    # Switch to switch links with 7ms latency
    net.addLink(s1, s2, cls=TCLink, delay='7ms')
    net.addLink(s2, s3, cls=TCLink, delay='7ms')
    net.addLink(s3, s4, cls=TCLink, delay='7ms')
    net.addLink(s4, s1, cls=TCLink, delay='7ms')
    net.addLink(s1, s3, cls=TCLink, delay='7ms')
    
    # Host to switch links with 5ms latency
    net.addLink(h1, s1, cls=TCLink, delay='5ms')
    net.addLink(h2, s1, cls=TCLink, delay='5ms')
    net.addLink(h3, s2, cls=TCLink, delay='5ms')
    net.addLink(h4, s2, cls=TCLink, delay='5ms')
    net.addLink(h5, s3, cls=TCLink, delay='5ms')
    net.addLink(h6, s3, cls=TCLink, delay='5ms')
    net.addLink(h7, s4, cls=TCLink, delay='5ms')
    net.addLink(h8, s4, cls=TCLink, delay='5ms')

    info('*** Starting network\n')
    net.build()
    
    info('*** Starting controllers\n')
    for controller in net.controllers:
        controller.start()

    info('*** Starting switches\n')
    net.get('s1').start([c0])
    net.get('s2').start([c0])
    net.get('s3').start([c0])
    net.get('s4').start([c0])

    info('*** Post configure switches and hosts\n')
    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    myNetwork()

