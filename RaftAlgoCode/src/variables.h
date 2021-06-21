// Variables for Configuration
#include "lib.h"


// NUMBER OF NODES IN THE NETWORK
int num_nodes = 6;

// PACKET PARAMETERS (not here...)
//int numRX           = 0;		// num of RX bytes in fifo
int packet_len      = 0;		// length of the received packet
int max_packet_len  = 0x14;		// maximum packet length allowed
//int fifo            = 0;		// variable for reading fifo
//char message[0x14];	// message string 

// IDENTIFIER PARAMETERS
int proposer_id     = 0;                        // ID of the proposer
int leader_id       = 0;                        // ID of the leader
int follower_ids[6] = {0};                  // IDs of nodes who follow you (proposer)
int network_ids[6]  = {0};                  // IDs of nodes that you can see 
int rssi_values[6]  = {127,0};                  // RSSI Values of nodes of network IDs
int global_network_ids[6] = {0};                // List of IDs the leader can see
int forwarder_ids[6] = {0};                     // List of IDs the node will forward

// TIMING PARAMETERS
//int msec = 0; // not here...
int timeout = 2000; //generate_random_timeout(); // not here..

// COUNTERS (maybe also not here)
//float accept_counter     = 0;
//float accept_not_counter = 0;

// BOOLEAN CHECK VALUES
//bool valid_packet = false; // not here...

// STATE VARIABLES

