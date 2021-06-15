#ifndef LIB_DOT_H    /* This is an "include guard" */
#define LIB_DOT_H    /* prevents the file from being included twice. */
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
//#include "ex2.h" 

// setup functions
void test();
void setIDLE();
void setRX();
int read_rssi1(int rssi_add);
void rssi_valid(int rssi_add);
void initialize_spi();
// state functions
bool state_open(int state);
bool state_follower(int state);
bool state_proposer(int state);
bool state_leader(int state);
int set_state_open();
int set_state_follower();
int set_state_proposer();
int set_state_leader();
// generating functions
int generate_random_id();
int generate_random_timout();
// message interaction
void send_message(int message_type, int tx_id, int rx_id);
int get_rx_id_from_msg(char *msg);
int get_tx_id_from_msg(char *msg);
int get_checksum_from_msg(char *msg);
char *get_type_from_message(char *msg);
int get_int_type_from_msg(char *msg);
int *get_broadcast_ids_from_msg(int *broadcast_network_ids, char *msg);
int compute_checksum(int message_type, int tx_id, int rx_id);
int *update_RSSI_list(int *rssi_values, int *network_ids, int sender_id, int rssi_value, int num_nodes);
void process_list_broadcast(int* global_network_ids, int len_global_network_ids, int* network_ids,int len_network_ids, int id);
// convert
int convert_char_to_int(char *msg);
// comparing
bool id_in_list(int *follower_ids, int id, int num_nodes);
bool valid_message(int message_type, int tx_id, int rx_id, int checksum);
#endif /* LIB_DOT_H */
