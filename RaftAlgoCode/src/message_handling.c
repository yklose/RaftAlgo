#include "message_handling.h"
#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
#include <unistd.h>
#include <stdbool.h>

// Variables just for message handling
int accept_not_counter = 0;
int accept_counter = 0;
int packet_len = 0;
int max_packet_len = 0x14;
int numRX = 0;
int fifo = 0;

int id;
int state;

void pass_id(int id_pass, int state_pass){
    id = id_pass;
    state = state_pass;
}

void tester(int a, int b){
    printf("done...\n");
}

int print_values(){
    printf("Test");
}

char *read_message(){
    char *message[20]; // TODO change!
    cc1200_reg_read(0x3F, &packet_len);
    // check if message is longer than expected
    if (packet_len>max_packet_len){
        packet_len = max_packet_len;
        printf("Transmitted message is longer than max configured lengths\n");
    }
    // read packet
    int k = 0;
    while(k<packet_len){
        cc1200_reg_read(0xD7,&numRX); // NUM RX BYTES = 0xD7
        if (numRX>0){
            cc1200_reg_read(0x3F, &fifo);
            message[k] = (char)fifo;
            k = k + 1;
        }
    }
    message[k+1] = '\0';
    printf("\nReceivedMessage: %s\n",message);
    return message;
}

void handle_propose_message(int sender_id, int proposer_id){
    printf("PROPOSE MESSAGE\n");

    if ((state_open(state))||(sender_id == proposer_id)){	
        printf("SEND ACCEPT OK message\n");
        state = set_state_follower();
        send_message(0x01, id, sender_id);
        proposer_id = sender_id;
    }
    else{
        printf("SEND ACCEPT DECLINE message\n");
        send_message(0x02, id, sender_id);
    }
}

void handle_accept_message(int sender_id){
    printf("ACCEPT OK MESSAGE\n");
    extern int num_nodes;
    extern int follower_ids[];
    if (state_proposer(state)){
        bool found = id_in_list(follower_ids, sender_id);
        if (found == false){
            printf("INCREASE ACCEPT COUNTER\n");
            accept_counter = accept_counter + 1;
            int n;
            // add sender_id to follower_ids
            for (n=0; n<num_nodes;++n){
                if (follower_ids[n]==0){
                    follower_ids[n]=sender_id;
                    break;
                }
            }
            printf("Anzahl ACCEPT Nachrichten: %f\n",accept_counter);
        }
        printf("partition of followers: %f\n",(accept_counter/num_nodes));
        // check if majority is reached
        if ((accept_counter/num_nodes)>0.5) { 
            printf("Clear Majority - SET LEADER\n");
            state = set_state_leader();
            accept_counter = 0;
            accept_not_counter = 0;
        }
    }
}


void handle_decline_message(int sender_id){
    printf("ACCEPT NOT MESSAGE\n");
    extern int num_nodes;
    extern int follower_ids[];
    // increase decline counter
    if (state_proposer(state)){
        bool found = id_in_list(follower_ids, sender_id);
        if (found == false){
            printf("INCREASE ACCEPT NOT COUNTER\n");
            accept_not_counter = accept_not_counter + 1;
            int n;
            // add sender_id to follower_ids
            for (n=0; n<num_nodes;++n){
                if (follower_ids[n]==0){
                    follower_ids[n]=sender_id;
                    break;
                }
            }
            printf("Anzahl ACCEPT NOT Nachrichten: %f\n",accept_not_counter);
        }
        // check if miniority is reached
        if ((accept_not_counter/num_nodes)>0.5) { 
            printf("Clear Minority - SET OPEN\n");
            state = set_state_open();
            accept_not_counter = 0;
            accept_counter = 0;
        }
    }
}

void handle_leader_message(int sender_id){
    printf("LEADER MESSAGE\n");
    extern int leader_id;
    // set state to follower
    state = set_state_follower();
    // send ok message
    send_message(0x04, id, sender_id);
    //save leader id
    leader_id = sender_id;
}

void handle_list_broadcast_message(int sender_id){
    printf("LIST_BROADCAST MESSAGE\n");
    // update global list 
    // if ids in local list not in global, send request

}

void handle_forward_ok_message(int sender_id){
    printf("FORWARD_OK MESSAGE\n");
    // check if message is for your id
    // add id to forwarder list

}

void read_incoming_packet_loop(void){
    // importing extern (global variables)
    extern int num_nodes;
    extern int network_ids[];
    extern int proposer_id;
    // stay in loop if not leader
    while(state_leader(state)==false){
		setRX();
		clock_t starttime = clock();
		int msec = update_msec(starttime);
		int timeout = 2000; //generate_random_timeout();
		bool valid_packet = false;
        int packet_len = 0;
		// Loop while no timeout
		while (msec < timeout){
			msec = update_msec(starttime);
			cc1200_reg_read(0xD7, &numRX); //NUM_RXBYTES = 0xD7
			if(numRX>0){
				printf("----------- PACKET detected -----------\n");
				rssi_valid(0x72);  //RSSI0 = 0x72
				int rssi = read_rssi1(0x71); //RSSI0 = 0x71
				printf("RSSI: %d\n", rssi);
                // process packet
				if (packet_len == 0){ 
					char message[packet_len] = read_message();
					// get message informations
					int sender_id = get_tx_id_from_msg(message);
					int receiver_id = get_rx_id_from_msg(message);
					int checksum = get_checksum_from_msg(message);
					char *sender_type = get_type_from_message(message);
					int sender_type_int = get_int_type_from_msg(message);
					bool checksum_correct = valid_message(sender_type_int, sender_id, receiver_id, checksum);
                    // print information for valid packet
					if (checksum_correct==true){
						printf("Sender Type: %s\n", sender_type);
						printf("tx_id: %d\n", sender_id);
						printf("rx_id: %d\n", receiver_id);
					}
					if (checksum_correct==true){
						// Update local list
						bool in_local_list = id_in_list(network_ids, sender_id);
						if (in_local_list == false){
							update_network_ids(sender_id, rssi);
							printf("Update Local list - new id\n");
						}
						else{
							update_rssi_list(sender_id, rssi);
							printf("Update Local list - new RSSI\n");
						}
						// evaluate message types
						if (strcmp(sender_type,"PROPOSE") == 0){
							handle_propose_message(sender_id, proposer_id);
                            valid_packet = true;
						}
						else if (strcmp(sender_type,"ACCEPT_OK") == 0){
							handle_accept_message(sender_id);
                            valid_packet = true;
						}
						else if (strcmp(sender_type,"ACCEPT_NOT") == 0){
							handle_decline_message(sender_id);
                            valid_packet = true;
						}
						else if (strcmp(sender_type,"LEADER") == 0){
							handle_leader_message(sender_id);
                            valid_packet = true;
						}
						else if (strcmp(sender_type,"LIST_BROADCAST") == 0){
							handle_list_broadcast_message(sender_id);
                            valid_packet = true;
						}
						else if (strcmp(sender_type,"FORWARD_OK") == 0){
                            handle_forward_ok_message(sender_id);
                            valid_packet = true;
						}
					}
					else{
						printf("invalid message\n");
					}
					// go in IDLE mode to Reset FIFO
					printf("\n\n");
					setIDLE();
					cc1200_cmd(SFRX);
					packet_len = 0;
					setRX();

					// Reset Timer
					printf("RESET TIMER\n");
					timeout = 2000; //generate_random_timeout();
					starttime = clock();
					break;
				}
			}
			if (state_leader(state)==true){
				break;
			}
		}
		// TIMER ABGELAUFEN (nur relevant für nicht leader!)
		if ((state_leader(state)==false) && (valid_packet==false)){ 
			printf("SEND PROPOSE\n");
			state = set_state_proposer();
			send_message(0x00, id, id);
		}
		// DEBUG deactivate! Otherwise keep!
		accept_counter = 0;
		accept_not_counter = 0;
		if (state_leader(state)==true){
			break;
		}
	}
    printf("Leaving Normal Loop - Entering Leader Loop...\n");
}