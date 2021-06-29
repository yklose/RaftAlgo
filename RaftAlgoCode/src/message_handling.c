#include "message_handling.h"
#include "lib.h" 
#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#define LEN(x)  (sizeof(x) / sizeof((x)[0]))

// global variables
float accept_not_counter = 0;
float accept_counter = 0;
int numRX = 0;
int fifo = 0;
extern int id;
extern int state;

// extern variables
extern int follower_ids[];
extern int network_ids[];
extern int rssi_values[];
extern int leader_id;
extern int proposer_id;
extern int num_nodes;
extern int packet_len;
extern int max_packet_len;

/*
void pass_global_values(int id_pass, int state_pass){
    id = id_pass;
    state = state_pass;
}*/

// try again in lib.c
int update_msec(int starttime){
        clock_t difference = clock()-starttime;
        int msec = difference * 1000 / CLOCKS_PER_SEC;
        return msec;
}


void update_rssi_list(int sender_id, int rssi){
        // update list
        int n;
        for (n=0; n<num_nodes;++n){
                if (network_ids[n]==sender_id){
                        rssi_values[n]=rssi;
                        break;
                }
        }      
}

void update_network_ids(int sender_id, int rssi){
        // add sender_id to network_ids
        int n=0;
        for (n=0; n<5;++n){  //TESTING 5 = num_nodes
			//printf("Network IDs N. %d", n);
				if (network_ids[n]==sender_id){
                        break;
                }
                if (network_ids[n]==0){
                        network_ids[n]=sender_id;
						printf("Update network id %d at %d\n",sender_id, n);
                        rssi_values[n]=rssi;
                        break;
                }
        }
}


char *read_message(){
    char *message = malloc(20); // TODO change!
    cc1200_reg_read(0x3F, &packet_len);
    // check if message is longer than expected
    if (packet_len>max_packet_len){
		printf("packet_len: %d\n", packet_len);
        packet_len = max_packet_len;
		
        printf("Transmitted message is longer than max configured lengths\n");
    }
    // read packet
    int k = 0;
    while(k<packet_len){
        cc1200_reg_read(0x2FD7,&numRX); // NUM RX BYTES = 0xD7
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
    // set state to follower
    state = set_state_follower();
    // send ok message
    send_message(0x04, id, sender_id);
    //save leader id
    leader_id = sender_id;
}

void handle_list_broadcast_message(char *msg){
    printf("LIST_BROADCAST MESSAGE\n");
    // update global list 
    // if ids in local list not in global, send request

	int num_ids = ((strlen(msg)-4)/7); //TODO: 7 to extern variable!
	int len_broadcast_network_ids = num_ids;
	//int broadcast_network_ids[len_broadcast_network_ids] = {0};
	int broadcast_network_ids[2] = {0}; // TODO: no hard coding
	// get_broadcast_ids_from_msg(broadcast_network_ids, msg, num_ids);
	get_broadcast_ids_from_msg(msg, broadcast_network_ids, 2, network_ids, 2, id); // TODO: no hard coding

	// process_list_broadcast(broadcast_network_ids, LEN(broadcast_network_ids), network_ids, 2, id);
}

void handle_forward_ok_message(int sender_id){
    printf("FORWARD_OK MESSAGE\n");
    // check if message is for your id
    // add id to forwarder list
	
}

void handle_request_forward_message(char *msg){
    printf("REQUEST FORWARD MESSAGE \n");
    
    int id_len = 7; // TODO
    int rssi_len = 3;
    char forwarder_id[7]; // TODO: id len
    char sender_id[7];
    char rssi[3];
    // Get forwarder ID
    int j;
    for (j=0; j<id_len; ++j){
        forwarder_id[j] = msg[1+j];
    }
    int forwarder_id_int = convert_char_to_int(forwarder_id);
    printf("Forwarder ID: %d\n", forwarder_id_int);
    // Get sender ID
    int k;
    for (k=0; k<id_len; ++k){
        sender_id[k] = msg[1+id_len+k];
    }
    int sender_id_int = convert_char_to_int(sender_id);
    printf("Sender ID: %d\n", sender_id_int);
    // Get RSSI
    int l;
    for (l=0; l<rssi_len; ++l){
        rssi[l] = msg[1+id_len+id_len+l];
    }
    int rssi_int = convert_char_to_int(rssi);
    printf("RSSI of Forwarder: %d\n", rssi_int);
    // TODO: ADD to potential new forwarder
    // later decide




}

void handle_ok_message(int sender_id){


}

void read_incoming_packet_loop(void){
    // stay in loop if not leader
    while(state_leader(state)==false){
		setRX();
		clock_t starttime   = clock();
		int msec            = update_msec(starttime);
		int timeout         = 2000; //generate_random_timeout();
        int packet_len      = 0;
		bool valid_packet   = false;
		// Loop while no timeout
		while (msec < timeout){
			msec = update_msec(starttime);
			cc1200_reg_read(0x2FD7, &numRX); //NUM_RXBYTES = 0xD7
			if(numRX>0){
				printf("----------- PACKET detected -----------\n");
				rssi_valid(0x2F72);  //RSSI0 = 0x72
				int rssi = read_rssi1(0x2F71); //RSSI0 = 0x71
				printf("RSSI: %d\n", rssi);
                // process packet
				if (packet_len == 0){ 
					char *message = read_message();
					// get message informations
					char *sender_type = get_type_from_message(message);
                    printf("Sender Type: %s\n", sender_type);

                
                    // TODO: check checksum!
					if (strcmp(sender_type,"LIST_BROADCAST") == 0){
						handle_list_broadcast_message(message);
						valid_packet = true;
					}
					else {
						int sender_id = get_tx_id_from_msg(message);
						int receiver_id = get_rx_id_from_msg(message);
						int checksum = get_checksum_from_msg(message);
						int sender_type_int = get_int_type_from_msg(message);
						bool checksum_correct = valid_message(sender_type_int, sender_id, receiver_id, checksum);
                        printf("Checksum: %d\n", checksum);
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
								printf("Update Local list - new rssi\n");
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
								// TESTING
								printf("LEADER MESSAGE\n");
								state = set_state_follower();
								//send_message(0x04, 2345678, sender_id);
								
								// TESTING END
								leader_id = sender_id;
								handle_leader_message(sender_id);
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
                printf("---------------------------------------\n");
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


void leader_loop(){
    int loop_counter = 0;
	setIDLE();
	cc1200_cmd(SFRX);
	packet_len = 0;
	setRX();
	bool broadcast_list_changed = true;
	// send_list_message(network_ids, num_nodes);
	// JUST FOR TESTING
	// printf("broadcast new list....\n");
	// send_list_message(network_ids, num_nodes);		
	// Leader Loop
	while (true){
		// print current state
		printf("Initialize Leader Loop \n\n");
		//	TESTIG
		


		setIDLE();
		cc1200_cmd(SFRX);
		setRX();
		// start clock
		clock_t starttime = clock();
        int msec = update_msec(starttime);
		int timeout = 500; //generate_random_timeout();
		bool valid_packet = false;
		bool heartbeat_send = false;
	
		int packet_len = 0;
		loop_counter++;
		// RX loop
		while (msec < timeout){
			// set up timer
			msec = update_msec(starttime);
			// read number of bytes in fifo
			cc1200_reg_read(0x2FD7, &numRX);
			// if there is a packet detected and you are not the leader!
			if(numRX>0){
				
				rssi_valid(0x2F72);
				if (packet_len == 0){ 
                    printf("----------- PACKET detected -----------\n");
                    int rssi = read_rssi1(0x2F71);
                    printf("RSSI: %d\n", rssi);
                    // read message
					// cc1200_reg_read(0x3F, &packet_len);
					// printf("packet_len: %d\n", packet_len);
					char *message = read_message();

					// get message informations
					char *sender_type = get_type_from_message(message);

                    // TODO: checksum
                    if (strcmp(sender_type,"FORWARD_REQUEST") == 0){
						handle_request_forward_message(message);
						valid_packet = true;
					}
                    else {

                        int sender_id = get_tx_id_from_msg(message);
                        int receiver_id = get_rx_id_from_msg(message);
                        int checksum = get_checksum_from_msg(message);
                        int sender_type_int = get_int_type_from_msg(message);
                        bool checksum_correct = valid_message(sender_type_int, sender_id, receiver_id, checksum);
                        
                        
                        
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
                                broadcast_list_changed = true;
                                printf("Update Local list - new id\n");
                            }
                            else{
                                update_rssi_list(sender_id, rssi);
                                printf("Update Local list - new rssi\n");
                            }
                            // evaluate message types
                            
                            if (strcmp(sender_type,"OK") == 0){
                                handle_ok_message(sender_id);
                                valid_packet = true;
                            }
                        }
                        else{
                            printf("invalid message\n");
                        }
                    }
					// go in IDLE mode to Reset FIFO
					printf("\n\n");
					setIDLE();
					cc1200_cmd(SFRX);
					packet_len = 0;
					setRX();
				}
			}
		}
	
		// check if local list changed?
		if ((broadcast_list_changed == true)&&(loop_counter%3==0)){
			printf("broadcast new list....\n");
			broadcast_list_changed = false;
			send_list_message(network_ids, num_nodes);
		}
        else if ((heartbeat_send == false)){
			send_message(0x03, id, id);
			printf("HEARTBEAT\n");
			heartbeat_send = true;
		}



	}

}