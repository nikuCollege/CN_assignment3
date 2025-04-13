Initially to run the mentioned tests, first execute the **nat_first.py** script.   


MININET COMMANDS -   
a) Test communication to an external host from an internal host:   
i) Ping to h5 from h1   
``` h1 ping -c 3 h5 ```   
ii) Ping to h3 from h2     
``` h2 ping -c 3 h3 ```   
b) Test communication to an internal host from an external host:    
i) Ping to h1 from h8    
``` h8 ping -c 3 h1 ```   
ii) Ping to h2 from h6   
``` h6 ping -c 3 h2 ```   
c) Iperf tests: 3 tests of 120s each.    
i) Run iperf3 server in h1 and iperf3 client in h6.   
``` h1 iperf3 -s & ```    
``` h6 iperf3 -c 172.16.10.10 -t 120 ```    
ii) Run iperf3 server in h8 and iperf3 client in h2.    
``` h2 iperf3 -s & ```    
``` h8 iperf3 -c 172.16.10.10 -t 120 ```   



Due to failed tests, reimplemented the logic with some changes.  

Executed the **nat_second.py** script.  

Re run the Miniet CLI tests.  

