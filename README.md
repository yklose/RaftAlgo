# Development of a leader election algorithm in a two hop network with respect to wireless environment

Wireless networks offer a wide range of communication possibilities and use cases. An important and essential part of this is the creation of a common consensus. This is often realized by a leader election, where a leader is determined after a predefined fair procedure. The leader then has certain privileges like the hosting and administration of databases etc. In this project we would like to implement this task in a wireless environment. Wireless environments have the great advantage that they do not require any infrastructure and are very flexible. Nevertheless, not only advantages occur with the implementation in wireless environments. In wireless environments, range or overloaded communication channels can lead to problems. In our project we would like to present a method how Beaglebones, equipped with a CC1200, can form a wireless network that autonomously performs a leader election and creating two hop system using forwarder mechanisms. 
The network should also function in the event of malfunctions and have redundancy against the failure of nodes. As soon as nodes fail, the network automatically regenerates itself and provides a new leader. 
If nodes are out of range of the leader, the messages are forwarded accordingly. To avoid overloading the communication channel, we have developed an alogrithm that performs RSSI-based forwarder determination. Since we do not have a central node acting as a server, our implementation is similar to a peer-to-peer network.



## Quickstart
In the following section the setup for the Beaglebone and CC1200 is described. The Beaglebone Black is from Texas Instruments and equipped with the Debian Linux operating system. The CC1200 (also by Texas Intruments) is used as a communication device that has a high performance RF transceiver included. In a first step the Beaglenbone has to be installed properly by using the instructions given on this website [Doku](https://kn-pr.tkn.tu-berlin.de/wiki/doku.php?id=network_protocol_programming_lab:environment_setup). For this project it is recommended to have 3 nodes, where each node is a Beaglebone with CC1200.

Once the Beaglebones are installed correctly clone the repository on each Beaglebone Black.

``git clone https://github.com/yklose/RaftAlgo``

In a next step the variable number of nodes has to be adjusted in the variables.h file. This is the only parameter that has to be adjusted for each scenario. In a standard configuration (with 3 nodes) the number of nodes is set by default to 3. After setting the parameter the code has to be compiled by executing:

``cd /build``

``cmake ../.``

``make``

Now everything is configured and each node can be started (at any time). To start the node please run the command:

``./raftalgo``
