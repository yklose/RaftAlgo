#ifndef MESSAGE_HANDLING_H    /* This is an "include guard" */
#define MESSAGE_HANDLING_H    /* prevents the file from being included twice. */
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>


int print_values();
void tester(int a, int b);
void pass_id(int id_pass, int state_pass);

char *read_message(void);
void handle_propose_message(int sender_id, int proposer_id);
void handle_accept_message(int sender_id);
void handle_decline_message(int sender_id);
void handle_leader_message(int sender_id);
void handle_list_broadcast_message(int sender_id);
void handle_forward_ok_message(int sender_id);
void read_incoming_packet_loop(void);



#endif /* MESSAGE_HANDLING_H */