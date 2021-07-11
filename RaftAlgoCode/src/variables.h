// Variables for Configuration
#include "lib.h"

// NUMBER OF NODES IN THE NETWORK
int num_nodes = 3;//6;
int id = 123;
int state = 0;
bool debug = false;

// PACKET PARAMETERS
int packet_len      = 0;		// length of the received packet
int max_packet_len  = 0x18;		// maximum packet length allowed
int id_len          = 6;
int decline_max     = 3;

// IDENTIFIER PARAMETERS
int proposer_id     = 0;                        // ID of the proposer
int leader_id       = 0;                        // ID of the leader
int follower_ids[6] = {0};                      // IDs of nodes who follow you (proposer)
int network_ids[6]  = {0};                      // IDs of nodes that you can see 
int rssi_values[6]  = {127,0};                  // RSSI Values of nodes of network IDs
int global_network_ids[6] = {0};                // List of IDs the leader can see
int forwarder_ids[6] = {0};                     // List of IDs the node will forward

// TIMING PARAMETERS (not here later)
//int timeout = 2000; //generate_random_timeout(); // not here..


