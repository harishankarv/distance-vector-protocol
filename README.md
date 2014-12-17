# Distance Vector routing protocol
A simplified  version of the Distance Vector Protocol using the Bellman Ford Algorithm (done as part of an assignment). The protocol runs on top of servers (behaving as routers) using UDP sockets.

Routing updates are exchanged periodically between neighboring servers based on a specific timeout interval specified at startup. When a new server sends routing  messages to its neighbors, the neighbous will add an entry in their routing tables corresponding to it. Servers can also be removed from a network. When a server has been removed from a network, it will no longer send distance vector updates to its neighbors. When a server no longer receives distance vector updates from its neighbor for 3 consecutive update intervals, it assumes that the neighbor no longer exists in the network and makes the appropriate changes to its routing table (link cost to this  neighbor will now be set to infinity but not remove it from the table). This information is propagated to other servers in the network with the exchange of routing updates.

Just `make` and run:  `./assignment3 ­-t <path­to­topology­file> -­i <routing­update­interval>`on multiple servers. Give the correct topology files, which are used for the initial configuration (sample topology files are given in the `./topology` folder):

```
//LINE ENTRY                    //COMMENTS
5                               //number of servers 
3                               //number of edges or neighbors 
1 128.205.36.8 4091             //server id 1 and corresponding IP, port pair 
2 128.205.35.24 4094            //server id 2 and corresponding IP, port pair    
3 128.205.36.24 4096            //server id 3 and corresponding IP, port pair 
4 128.205.36.4 7091             //server id 4 and corresponding IP, port pair 
5 128.205.36.25 7864            //server id 5 and corresponding IP, port pair
1 2 7                           //server id and neighbor id and cost 
1 3 4                           //server id and neighbor id and cost 
1 4 5                           //server id and neighbor id and cost 
```
