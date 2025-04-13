Steps to execute :
```sudo python3 network_topology.py```

In the mininet CLI, execute these commands : 
- Ping h1 from h3
  ```h3 ping h1 -c 10```
- Ping h7 from h5
  ```h5 ping h7 -c 10```
- Ping h2 from h8
  ```h8 ping h2 -c 10```  

To fix the problem of unsuccessful pings , implementing SPANNING TREE PROTOCOL : 
- ```sh ovs-vsctl set bridge s1 stp_enable=true```  
- ```sh ovs-vsctl set bridge s2 stp_enable=true```  
- ```sh ovs-vsctl set bridge s3 stp_enable=true```  
- ```sh ovs-vsctl set bridge s4 stp_enable=true```   

To visualise the effect on switches :  
- ```sh ovs-ofctl dump-flows s1```   
- ```sh ovs-ofctl dump-flows s2```  
- ```sh ovs-ofctl dump-flows s3```  
- ```sh ovs-ofctl dump-flows s4```  
