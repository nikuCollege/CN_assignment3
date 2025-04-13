#!/usr/bin/python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Node
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from mininet.link import TCLink
import time
import os

class LinuxRouter(Node):
    """A Node with IP forwarding enabled."""
    def config(self, **params):
        super(LinuxRouter, self).config(**params)
        self.cmd('sysctl net.ipv4.ip_forward=1')

    def terminate(self):
        self.cmd('sysctl net.ipv4.ip_forward=0')
        super(LinuxRouter, self).terminate()

class NATTopo(Topo):
    def build(self, **_opts):
        # Switches
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')

        # Public network hosts (172.16.10.0/24)
        h3 = self.addHost('h3', ip='172.16.10.4/24')
        h4 = self.addHost('h4', ip='172.16.10.5/24')
        h5 = self.addHost('h5', ip='172.16.10.6/24')
        h6 = self.addHost('h6', ip='172.16.10.7/24')
        h7 = self.addHost('h7', ip='172.16.10.8/24')
        h8 = self.addHost('h8', ip='172.16.10.9/24')

        # NAT Router
        nat = self.addNode('h9', cls=LinuxRouter, ip='172.16.10.10/24')

        # Private network hosts (10.1.1.0/24)
        h1 = self.addHost('h1', ip='10.1.1.2/24', defaultRoute='via 10.1.1.1')
        h2 = self.addHost('h2', ip='10.1.1.3/24', defaultRoute='via 10.1.1.1')

        # Switch links with 7ms delay
        self.addLink(s1, s2, cls=TCLink, delay='7ms')
        self.addLink(s2, s3, cls=TCLink, delay='7ms')
        self.addLink(s3, s4, cls=TCLink, delay='7ms')
        self.addLink(s4, s1, cls=TCLink, delay='7ms')
        self.addLink(s1, s3, cls=TCLink, delay='7ms')

        # Host links with 5ms delay
        self.addLink(h3, s2, cls=TCLink, delay='5ms')
        self.addLink(h4, s2, cls=TCLink, delay='5ms')
        self.addLink(h5, s3, cls=TCLink, delay='5ms')
        self.addLink(h6, s3, cls=TCLink, delay='5ms')
        self.addLink(h7, s4, cls=TCLink, delay='5ms')
        self.addLink(h8, s4, cls=TCLink, delay='5ms')

        # NAT connections
        self.addLink(nat, s1, intfName1='h9-eth0', cls=TCLink, delay='5ms')
        self.addLink(h1, nat, intfName2='h9-eth1', cls=TCLink, delay='5ms',
                     params2={'ip': '10.1.1.1/24'})
        self.addLink(h2, nat, intfName2='h9-eth2', cls=TCLink, delay='5ms',
                     params2={'ip': '10.1.1.1/24'})

def configure_nat(net):
    """Configure NAT functionality on host h9"""
    h9 = net.get('h9')
    
    # Clear any existing iptables rules
    h9.cmd('iptables -F')
    h9.cmd('iptables -t nat -F')
    
    # NAT configuration - MASQUERADE for outgoing traffic
    h9.cmd('iptables -t nat -A POSTROUTING -s 10.1.1.0/24 -o h9-eth0 -j MASQUERADE')
    
    # Forwarding rules - allow established connections and new connections
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth1 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth1 -o h9-eth0 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth2 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth2 -o h9-eth0 -j ACCEPT')
    
    # Optional: Allow traffic to/from internal hosts for specific services
    # This would be needed for incoming connections to the private network:
    # For example, to allow external host to connect to h1 on port 5201 (iperf3):
    h9.cmd('iptables -t nat -A PREROUTING -i h9-eth0 -p tcp --dport 5201 -j DNAT --to-destination 10.1.1.2:5201')
    
    # Verify NAT configuration
    info('\n*** NAT Configuration on h9:\n')
    info(h9.cmd('iptables -t nat -L -n -v'))
    info(h9.cmd('iptables -L -n -v'))

def test_connectivity(net):
    """Test connectivity between hosts"""
    info('\n*** Testing internal to external connectivity\n')
    h1, h5 = net.get('h1', 'h5')
    info('Ping from h1 (internal) to h5 (external):\n')
    info(h1.cmd('ping -c 3 172.16.10.6'))
    
    h2, h3 = net.get('h2', 'h3')
    info('Ping from h2 (internal) to h3 (external):\n')
    info(h2.cmd('ping -c 3 172.16.10.4'))
    
    info('\n*** Testing external to internal connectivity\n')
    h8, h1 = net.get('h8', 'h1')
    info('Ping from h8 (external) to h1 (internal):\n')
    # This will likely fail without specific port forwarding rules
    info(h8.cmd('ping -c 3 10.1.1.2'))
    
    h6, h2 = net.get('h6', 'h2')
    info('Ping from h6 (external) to h2 (internal):\n')
    # This will likely fail without specific port forwarding rules
    info(h6.cmd('ping -c 3 10.1.1.3'))

def run_iperf_tests(net):
    """Run iperf tests as specified in the requirements"""
    info('\n*** Setting up iperf tests\n')
    
    # Ensure iperf3 is installed
    os.system('which iperf3 > /dev/null || apt-get install -y iperf3')
    
    # Test 1: h1 server, h6 client
    h1, h6 = net.get('h1', 'h6')
    
    # Start iperf server on h1
    h1.cmd('iperf3 -s -D')
    info('Started iperf3 server on h1\n')
    
    # Add port forwarding for iperf (port 5201)
    nat = net.get('h9')
    nat.cmd('iptables -t nat -A PREROUTING -i h9-eth0 -p tcp --dport 5201 -j DNAT --to-destination 10.1.1.2:5201')
    
    # Test from h6 to h1 (via NAT)
    info('Running iperf3 test from h6 to h1 (120s):\n')
    info(h6.cmd('iperf3 -c 172.16.10.10 -t 120'))
    
    # Stop server on h1
    h1.cmd('pkill -f "iperf3 -s"')
    
    # Test 2: h8 server, h2 client
    h8, h2 = net.get('h8', 'h2')
    
    # Start iperf server on h8
    h8.cmd('iperf3 -s -D')
    info('Started iperf3 server on h8\n')
    
    # Test from h2 to h8
    info('Running iperf3 test from h2 to h8 (120s):\n')
    info(h2.cmd('iperf3 -c 172.16.10.9 -t 120'))
    
    # Stop server on h8
    h8.cmd('pkill -f "iperf3 -s"')

def start_network():
    """Start the network and run tests"""
    setLogLevel('info')
    topo = NATTopo()
    net = Mininet(topo=topo, link=TCLink)
    net.start()
    
    info('\n*** Network started\n')
    
    # Configure public host routes
    for h in ['h3', 'h4', 'h5', 'h6', 'h7', 'h8']:
        host = net.get(h)
        host.cmd('ip route add default via 172.16.10.10')
        host.cmd('ip route add 10.1.1.0/24 via 172.16.10.10')
        info(f'Configured routes on {h}\n')
    
    # Configure NAT on h9
    configure_nat(net)
    
    # Test basic connectivity
    test_connectivity(net)
    
    # Option to run iperf tests (commented out by default as they take time)
    # info('\n*** To run iperf tests, uncomment the run_iperf_tests function call in the script\n')
    # run_iperf_tests(net)
    
    info('\n*** To run iperf tests manually:\n')
    info('1. On h1: iperf3 -s\n')
    info('2. On h6: iperf3 -c 172.16.10.10 -t 120\n')
    info('3. On h8: iperf3 -s\n')
    info('4. On h2: iperf3 -c 172.16.10.9 -t 120\n')
    
    CLI(net)
    net.stop()

if __name__ == '__main__':
    start_network()
