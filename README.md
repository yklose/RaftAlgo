# Development of a leader election algorithm in a two hop network with respect to wireless environment

Wireless networks offer a wide range of communication possibilities and use cases. An important and essential part of this is the creation of a common consensus. This is often realized by a leader election, where a leader is determined after a predefined fair procedure. The leader then has certain privileges like the hosting and administration of databases etc. In this project we would like to implement this task in a wireless environment. Wireless environments have the great advantage that they do not require any infrastructure and are very flexible. Nevertheless, not only advantages occur with the implementation in wireless environments. In wireless environments, range or overloaded communication channels can lead to problems. In our project we would like to present a method how Beaglebones, equipped with a CC1200, can form a wireless network that autonomously performs a leader election and creating two hop system using forwarder mechanisms. 
The network should also function in the event of malfunctions and have redundancy against the failure of nodes. As soon as nodes fail, the network automatically regenerates itself and provides a new leader. 
If nodes are out of range of the leader, the messages are forwarded accordingly. To avoid overloading the communication channel, we have developed an alogrithm that performs RSSI-based forwarder determination. Since we do not have a central node acting as a server, our implementation is similar to a peer-to-peer network.
