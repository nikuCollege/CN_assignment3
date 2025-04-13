#!/usr/bin/python
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Node
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from mininet.link import TCLink
import time

class LinuxRouter(Node):
    def config(self, **params):
        super(LinuxRouter, self).config(**params)
        self.cmd('sysctl net.ipv4.ip_forward=1')
        
    def terminate(self):
        self.cmd('sysctl net.ipv4.ip_forward=0')
        super(LinuxRouter, self).terminate()

class NATTopo(Topo):
    def build(self):
        # Add switches
        s1, s2, s3, s4 = [self.addSwitch(s) for s in ('s1','s2','s3','s4')]
        
        # Connect switches in a ring topology
        self.addLink(s1, s2, cls=TCLink, delay='7ms')
        self.addLink(s2, s3, cls=TCLink, delay='7ms')
        self.addLink(s3, s4, cls=TCLink, delay='7ms')
        self.addLink(s4, s1, cls=TCLink, delay='7ms')
        
        # Public network (172.16.10.0/24)
        hosts = {
            'h3': ('s2', '172.16.10.4/24', '172.16.10.1'),
            'h4': ('s2', '172.16.10.5/24', '172.16.10.1'),
            'h5': ('s3', '172.16.10.6/24', '172.16.10.1'),
            'h6': ('s3', '172.16.10.7/24', '172.16.10.1'),
            'h7': ('s4', '172.16.10.8/24', '172.16.10.1'),
            'h8': ('s4', '172.16.10.9/24', '172.16.10.1')
        }
        
        # Add NAT router
        nat = self.addNode('h9', cls=LinuxRouter, ip='172.16.10.10/24')
        
        # Private network (10.1.1.0/24)
        self.addHost('h1', ip='10.1.1.2/24', defaultRoute='via 10.1.1.1')
        self.addHost('h2', ip='10.1.1.3/24', defaultRoute='via 10.1.1.1')
        
        # Add public hosts with default routes
        for host, (switch, ip, default_gw) in hosts.items():
            self.addHost(host, ip=ip, defaultRoute=f'via {default_gw}')
            self.addLink(host, switch, cls=TCLink, delay='5ms')
        
        # Add NAT connections - connect to s1 and private hosts
        self.addLink(nat, s1, intfName1='h9-eth0', params1={'ip': '172.16.10.10/24'})
        self.addLink('h1', nat, intfName2='h9-eth1', params2={'ip': '10.1.1.1/24'})
        self.addLink('h2', nat, intfName2='h9-eth2', params2={'ip': '10.1.1.1/24'})

def configure_nat(net):
    h9 = net.get('h9')
    
    # Clear existing rules
    h9.cmd('iptables -F')
    h9.cmd('iptables -t nat -F')
    h9.cmd('iptables -t mangle -F')
    
    # Enable IP forwarding (redundant but safe)
    h9.cmd('echo 1 > /proc/sys/net/ipv4/ip_forward')
    
    # Basic NAT - Masquerade outgoing connections
    h9.cmd('iptables -t nat -A POSTROUTING -o h9-eth0 -s 10.1.1.0/24 -j MASQUERADE')
    
    # Port forwarding for iperf
    h9.cmd('iptables -t nat -A PREROUTING -i h9-eth0 -p tcp --dport 5201 -j DNAT --to-destination 10.1.1.2:5201')
    h9.cmd('iptables -t nat -A PREROUTING -i h9-eth0 -p tcp --dport 5202 -j DNAT --to-destination 10.1.1.3:5202')
    
    # Allow forwarding
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth1 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth1 -o h9-eth0 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth2 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth2 -o h9-eth0 -j ACCEPT')
    
    # Additional rules to ensure connections work
    h9.cmd('iptables -A FORWARD -j ACCEPT')  # Accept all forwarded traffic
    
    # Add routes for NAT router
    h9.cmd('ip route add 10.1.1.0/24 dev h9-eth1')
    h9.cmd('ip route add default via 172.16.10.1')
    
    # Debug - show routing and rules
    info('*** NAT router routes: \n')
    info(h9.cmd('route -n'))
    info('*** NAT rules: \n')
    info(h9.cmd('iptables -t nat -L -v -n'))

def configure_routes(net):
    # Configure gateway on switches (optional)
    for i, s in enumerate(['s1', 's2', 's3', 's4']):
        switch = net.get(s)
        switch.cmd(f'ifconfig {s} 172.16.10.{100+i}/24')
    
    # Configure public hosts
    for h in ['h3', 'h4', 'h5', 'h6', 'h7', 'h8']:
        host = net.get(h)
        # Make sure all public hosts can reach the NAT
        host.cmd('ip route add 10.1.1.0/24 via 172.16.10.10')
        host.cmd('ip route add default via 172.16.10.10')
        
        # Display routes for debugging
        info(f'*** Routes for {h}: \n')
        info(host.cmd('route -n'))
    
    # Configure interfaces for private hosts
    for h in ['h1', 'h2']:
        host = net.get(h)
        info(f'*** Routes for {h}: \n')
        info(host.cmd('route -n'))

def run_tests(net):
    h1, h2, h3, h5, h6, h8 = net.get('h1', 'h2', 'h3', 'h5', 'h6', 'h8')
    
    # Debugging - check connectivity
    info('*** Checking network connectivity\n')
    
    # Test router connectivity
    info('h9 routes:\n' + net.get('h9').cmd('ip route'))
    info('h9 interfaces:\n' + net.get('h9').cmd('ifconfig'))
    
    # Test ping between hosts on the same subnet
    info('h1 → h2 (same subnet):\n' + h1.cmd('ping -c2 10.1.1.3'))
    
    # Test ping from private to public
    info('h1 → h3 (private to public):\n' + h1.cmd('ping -c2 172.16.10.4'))
    
    # Test ping from public to NAT
    info('h3 → h9 (public to NAT):\n' + h3.cmd('ping -c2 172.16.10.10'))
    
    # Test ping from public to private (should be translated)
    info('h3 → h1 (public to private):\n' + h3.cmd('ping -c2 10.1.1.2'))
    
    # Original test pings
    info('h2 → h3:\n' + h2.cmd('ping -c3 172.16.10.4'))
    info('h6 → h2:\n' + h6.cmd('ping -c3 10.1.1.3'))
    
    # Set up iperf servers
    h1.cmd('iperf3 -s -p 5201 &')
    h2.cmd('iperf3 -s -p 5202 &')
    h8.cmd('iperf3 -s -p 5203 &')
    
    # Wait for iperf servers to start
    time.sleep(1)
    
    # Run iperf tests
    info('h6 → h1 (through NAT):\n' + h6.cmd('iperf3 -c 172.16.10.10 -p 5201 -t 5'))
    info('h3 → h2 (through NAT):\n' + h3.cmd('iperf3 -c 172.16.10.10 -p 5202 -t 5'))
    info('h2 → h8 (private to public):\n' + h2.cmd('iperf3 -c 172.16.10.9 -p 5203 -t 5'))

def start_network():
    setLogLevel('info')
    
    # Create network with given topology
    topo = NATTopo()
    net = Mininet(topo=topo, link=TCLink)
    net.start()
    
    # Configure STP on switches
    info('*** Configuring Spanning Tree Protocol\n')
    for s in net.switches:
        info(f'Setting STP on {s}\n')
        s.cmd('ovs-vsctl set bridge %s stp_enable=true' % s)
    
    # Wait for STP to converge
    info('*** Waiting for STP to converge (15s)\n')
    time.sleep(30)  # Reduced wait time but should be enough
    
    # Configure NAT
    info('*** Configuring NAT\n')
    configure_nat(net)
    
    # Configure routes
    info('*** Configuring routes\n')
    configure_routes(net)
    
    # Run connectivity tests
    info('*** Running connectivity tests\n')
    run_tests(net)
    
    # Start CLI
    CLI(net)
    
    # Clean up
    net.stop()

if __name__ == '__main__':
    start_network()
