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
float accept_counter = 1;
int numRX = 0;
int fifo = 0;
extern int id;
extern int state;
int decline_counter = 0;
extern int debug;

// extern variables
extern int follower_ids[];
extern int network_ids[];
extern int rssi_values[];
extern int forwarder_ids[];
extern int leader_id;
extern int proposer_id;
extern int num_nodes;
extern int packet_len;
extern int max_packet_len;

// later extern
int potential_sender_ids[6] = {0};
int potential_forwarder_ids[6] = {0};
int potential_forwarder_rssi[6] = {0};

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
                        if (debug == true){
						    printf("Update network id %d at %d\n",sender_id, n);
                        }
                        rssi_values[n]=rssi;
                        break;
                }
        }
}

void update_forwarder_ids(int forwarder){
        // add sender_id to network_ids
        int n=0;
        for (n=0; n<5;++n){  //TESTING 5 = num_nodes
            if (forwarder_ids[n]==0){
                    forwarder_ids[n]=forwarder;
                    printf("HAVE TO FORWARD: %d\n", forwarder_ids[n]);
                    break;
            }
        }
}

void update_potential_lists(int sender_id, int forwarder_id, int rssi){
        int n=0;
        for (n=0; n<6;++n){  //TESTING 5 = num_nodes
                if (potential_forwarder_ids[n]==0){
                        potential_forwarder_ids[n]=forwarder_id;
                        potential_forwarder_rssi[n]=rssi;
                        potential_sender_ids[n]=sender_id;
                        if (debug == true){
                            printf("Add to potential list (Toforward, rssi, sender) : %d, %d, %d\n", potential_forwarder_ids[n], potential_forwarder_rssi[n], potential_sender_ids[n]);
                        }
                        break;
                }
        }
}

char *read_message(){
    char *message = malloc(20); // TODO change!
    cc1200_reg_read(0x3F, &packet_len);
    // check if message is longer than expected
    if (packet_len>max_packet_len){
        if (debug == true){
		    printf("packet_len: %d\n", packet_len);
            printf("Transmitted message is longer than max configured lengths\n");
        }
        packet_len = max_packet_len;
		
        
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
    if (debug == true){
        printf("\nReceivedMessage: %s\n",message);
    }
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
        decline_counter += 1;
        if (decline_counter < 5) {
            send_message(0x02, id, sender_id);
        }
        else{
            printf("Too many decline msg, set open...\n");
            state = set_state_open();
            decline_counter = 0;
        }
    }
}

void handle_accept_message(int sender_id, int receiver_id){
    printf("ACCEPT OK MESSAGE\n");
    if (receiver_id == id){
        if (state_proposer(state)){
            bool found = id_in_list(follower_ids, sender_id);
            if (found == false){
                if (debug == true){
                    printf("INCREASE ACCEPT COUNTER\n");
                }
                accept_counter = accept_counter + 1;
                int n;
                // add sender_id to follower_ids
                for (n=0; n<num_nodes;++n){
                    if (follower_ids[n]==0){
                        follower_ids[n]=sender_id;
                        break;
                    }
                }
                if (debug == true){
                    printf("Anzahl ACCEPT Nachrichten: %f\n",accept_counter);
                }
            }
            printf("partition of followers: %f\n",(accept_counter/num_nodes));
            // check if majority is reached
            if ((accept_counter/num_nodes)>0.5) { 
                printf("Clear Majority - SET LEADER\n");
                state = set_state_leader();
                accept_counter = 1;
                accept_not_counter = 0;
            }
        }
    }
}


void handle_decline_message(int sender_id, int receiver_id){
    printf("ACCEPT NOT MESSAGE\n");
    // increase decline counter
    if (receiver_id == id){
        if (state_proposer(state)){
            bool found = id_in_list(follower_ids, sender_id);
            if (found == false){
                if (debug == true){
                    printf("INCREASE ACCEPT NOT COUNTER\n");
                }
                accept_not_counter = accept_not_counter + 1;
                int n;
                // add sender_id to follower_ids
                for (n=0; n<num_nodes;++n){
                    if (follower_ids[n]==0){
                        follower_ids[n]=sender_id;
                        break;
                    }
                }
                if (debug == true){
                    printf("Anzahl ACCEPT NOT Nachrichten: %f\n",accept_not_counter);
                }
            }
            // check if miniority is reached
            if ((accept_not_counter/num_nodes)>0.5) { 
                printf("Clear Minority - SET OPEN\n");
                state = set_state_open();
                accept_not_counter = 0;
                accept_counter = 1;
            }
        }
    }
}

void handle_leader_message(int sender_id){
    printf("LEADER MESSAGE\n");
    // set state to follower
    state = set_state_follower();
    // save leader id
    leader_id = sender_id;
    send_message(0x04,leader_id,id);

}

void handle_list_broadcast_message(char *msg){
    printf("LIST_BROADCAST MESSAGE\n");
    // update global list 
    // if ids in local list not in global, send request

	int num_ids = ((strlen(msg)-4)/6); //TODO: 7 to extern variable!
	int len_broadcast_network_ids = num_ids;
	//int broadcast_network_ids[len_broadcast_network_ids] = {0};
	// int broadcast_network_ids[2] = {0}; // TODO: no hard coding
	// get_broadcast_ids_from_msg(broadcast_network_ids, msg, num_ids);
	get_broadcast_ids_from_msg(msg, broadcast_network_ids, 2, network_ids, 2, id); // TODO: no hard coding

	// process_list_broadcast(broadcast_network_ids, LEN(broadcast_network_ids), network_ids, 2, id);
}

void handle_forward_ok_message(int forwarder_id, int receiver_id){
    printf("FORWARD_OK MESSAGE\n");
    if (id == receiver_id){
        if (debug == true){
            printf("UPDATE FORWARDER IDS\n");
        }
        update_forwarder_ids(forwarder_id);
    }

    // check if message is for your id
    // add id to forwarder list
	
}

void delete_list(int *array, int lenght){
    int j;
    for (j=0; j<lenght; ++j){
        array[j] = 0;
    }
}

void choose_forwarder(){
    int i;
    int current_forward_id; 
    int current_forwarder_rssi; 
    int current_winner;
    int used[6] = {false};
    for (i=0; i<6; ++i){
        if ((used[i]==false)&&(potential_forwarder_ids[i]!=0)){
            current_forward_id = potential_forwarder_ids[i];
            current_forwarder_rssi = potential_forwarder_rssi[i];
            current_winner = potential_sender_ids[i];
            int j;
            for (j=i; j<6; ++j){
                if (potential_forwarder_ids[j] == current_forward_id){
                    used[j] = true;
                    if (potential_forwarder_rssi[j] > current_forwarder_rssi){
                        current_winner = potential_sender_ids[j];
                        current_forwarder_rssi = potential_forwarder_rssi[j];
                    }
                }
            }
            // send 
            printf("Chose forwarder\n");
            send_message(0x06, current_forward_id, current_winner);
        }
    }
    delete_list(potential_forwarder_ids, 6);
    delete_list(potential_forwarder_rssi, 6);
    delete_list(potential_sender_ids, 6);
}



void handle_request_forward_message(char *msg){
    printf("REQUEST FORWARD MESSAGE \n");
    extern int id_len; // TODO
    int rssi_len = 3;
    char forwarder_id[6]; // TODO: id len
    char sender_id[6];
    char rssi[3];
    // Get forwarder ID
    int j;
    for (j=0; j<id_len; ++j){
        forwarder_id[j] = msg[1+j];
    }
    int forwarder_id_int = convert_char_to_int(forwarder_id);
    if (debug == true){
        printf("Forwarder ID: %d\n", forwarder_id_int);
    }
    // Get sender ID
    int k;
    for (k=0; k<id_len; ++k){
        sender_id[k] = msg[1+id_len+k];
    }
    int sender_id_int = convert_char_to_int(sender_id);
    if (debug == true){
        printf("Sender ID: %d\n", sender_id_int);
    }
    // Get RSSI
    int l;
    for (l=0; l<rssi_len; ++l){
        rssi[l] = msg[1+id_len+id_len+l];
    }
    int rssi_int = convert_char_to_int(rssi);
    if (debug == true){
        printf("RSSI of Forwarder: %d\n", rssi_int);
    }
    // TODO: ADD to potential new forwarder
    // later decide
    update_potential_lists(sender_id_int, forwarder_id_int, rssi_int);
    //update_potential_lists(222222, forwarder_id_int, 20); 
    //update_potential_lists(333333, 555555, 45);

    choose_forwarder();

}





void handle_ok_message(int sender_id){
    // handle ok messages


}

void read_incoming_packet_loop(void){
    // stay in loop if not leader
    while(state_leader(state)==false){
		setRX();
		clock_t starttime   = clock();
		int msec            = update_msec(starttime);
		int timeout         = generate_random_timeout();
        int packet_len      = 0;
		bool valid_packet   = false;
        int heart_beat      = 0;
		// Loop while no timeout
		while (msec < timeout){
			msec = update_msec(starttime);
			cc1200_reg_read(0x2FD7, &numRX); //NUM_RXBYTES = 0xD7
			if(numRX>0){
                if (debug == true){
				    printf("----------- PACKET detected -----------\n");
                }
				rssi_valid(0x2F72);  //RSSI0 = 0x72
				int rssi = read_rssi1(0x2F71); //RSSI0 = 0x71
				printf("RSSI: %d\n", rssi);
                // process packet
				if (packet_len == 0){ 
                    if (debug == true){
                        printf("CLOCK - receive: %d\n",clock()* 1000/CLOCKS_PER_SEC);
                    }
					char *message = read_message();
					// get message informations
					char *sender_type = get_type_from_message(message);
                    int checksum = get_checksum_from_msg(message);
                    if (debug == true){
                        printf("Sender Type: %s\n", sender_type);
                        printf("Checksum: %d\n", checksum);
                    }

                    // Look for datatype 2 (list broadcast)
					if (strcmp(sender_type,"LIST_BROADCAST") == 0){
                        bool checksum_correct = valid_list_message(checksum);
                        if (checksum_correct==true){
						    handle_list_broadcast_message(message);
						    valid_packet = true;
                        }
                        else{
                            printf("invalid message\n");
                        }
					}

                    // Look for datatype 1 (Propose, Accept, Decline, Leader, Forward ok)
					else {
						int sender_id = get_tx_id_from_msg(message);
						int receiver_id = get_rx_id_from_msg(message);
						int sender_type_int = get_int_type_from_msg(message);
						bool checksum_correct = valid_message(sender_type_int, sender_id, receiver_id, checksum);
                        if (debug == true){
                            printf("sender_type_int: %d\n", sender_type_int);
                            printf("checksum: %d\n", checksum);
                            printf("tx_id: %d\n", sender_id);
                            printf("rx_id: %d\n", receiver_id);
                        }
						if (checksum_correct==true){

							// Update local list
							bool in_local_list = id_in_list(network_ids, sender_id);
							if (in_local_list == false){
								update_network_ids(sender_id, rssi);
                                if (debug == true){
								    printf("Update Local list - new id\n");
                                }
							}
							else{
								update_rssi_list(sender_id, rssi);
                                if (debug == true){
								    printf("Update Local list - new rssi\n");
                                }
							}

                            // Check if message is for me or for forwarder
                            bool msg_for_me = (receiver_id == id);
                            bool msg_to_forward = id_in_list(forwarder_ids, receiver_id);
                            bool msg_from_forward = id_in_list(forwarder_ids, sender_id);
                            bool broadcast = (receiver_id==sender_id);
                            if (debug == true){
                                printf("for me %d, to_forward %d, from_forward %d, broadcast %d\n", msg_for_me,msg_to_forward,msg_from_forward, broadcast);
                            }

                            // Forward Messages and Broadcast to to_forward node
                            if ((msg_to_forward==true)||(msg_from_forward==true)||(receiver_id==sender_id)){
                                if ((msg_for_me == false)&&(forwarder_ids[0]!=0)){
                                    printf("FORWARD MESSAGE\n");
                                    send_message(sender_type_int, sender_id, receiver_id);
                                }
                            }

                            if ((msg_for_me==true)||(broadcast==true)){
                                // evaluate message types
                                if (strcmp(sender_type,"PROPOSE") == 0){
                                    handle_propose_message(sender_id, proposer_id);
                                    valid_packet = true;
                                }
                                else if (strcmp(sender_type,"ACCEPT_OK") == 0){
                                    handle_accept_message(sender_id, receiver_id);
                                    valid_packet = true;
                                }
                                else if (strcmp(sender_type,"ACCEPT_NOT") == 0){
                                    handle_decline_message(sender_id, receiver_id);
                                    valid_packet = true;
                                }
                                else if (strcmp(sender_type,"LEADER") == 0){
                                    handle_leader_message(sender_id);
                                    valid_packet = true;
                                    heart_beat = 0;
                                }
                                else if (strcmp(sender_type,"FORWARD_OK") == 0){
                                    handle_forward_ok_message(sender_id, receiver_id);
                                    valid_packet = true;
                                }
                            }
						}
						else{
							printf("invalid message\n");
						}
					}
                    
					// go in IDLE mode to Reset FIFO (in function)
					printf("\n");
					setIDLE();
					cc1200_cmd(SFRX);
					packet_len = 0;
					setRX();
                    if (debug == true){
                        printf("CLOCK - END: %d\n",clock()* 1000/CLOCKS_PER_SEC);
                    }

                    heart_beat += msec;
					// Reset Timer
                    if (debug == true){
					    printf("RESET TIMER\n");
                    }
					timeout = generate_random_timeout();
					starttime = clock();

                    if (debug == true){
                        printf("HEARTBEAT: %d\n", heart_beat);
                    }
                    if ((state_follower(state)==true)&&(heart_beat>1000)){
                        if (debug == true){
                            printf("SET OPEN\n");
                        }
                        state = set_state_open();
                    }
					break;
				}
                if (debug == true){
                    printf("---------------------------------------\n");
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
		// DEBUG deactivate! Otherwise keep! (function for reset variables)
		accept_counter = 1;
		accept_not_counter = 0;
        proposer_id = 0;                       
        leader_id = 0;
        //RESET follower ids
        int i;
        for (i = 0; i<6; ++i){
            follower_ids[i] = 0;
        }
        

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
    int i;
    for (i = 0; i<6; ++i){
        network_ids[i] = 0;
        rssi_values[i] = 0;
    }
    network_ids[0] = id;
    rssi_values[0] = 127;
	// send_list_message(network_ids, num_nodes);
	// JUST FOR TESTING
	// printf("broadcast new list....\n");
	// send_list_message(network_ids, num_nodes);		
	// Leader Loop
	while (true){
		// print current state
		//printf("Initialize Leader Loop \n\n");
		//	TESTIG
		


		setIDLE();
		cc1200_cmd(SFRX);
		setRX();
		// start clock
		clock_t starttime = clock();
        int msec = update_msec(starttime);
		int timeout = 300; //generate_random_timeout();
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
                    if (debug == true){
                        printf("CLOCK - receive: %d\n",clock()* 1000/CLOCKS_PER_SEC);
                        printf("----------- PACKET detected -----------\n");
                    }
                    int rssi = read_rssi1(0x2F71);
                    if (debug == true){
                        printf("RSSI: %d\n", rssi);
                    }
                    // read message
					char *message = read_message();

					// get message informations
					char *sender_type = get_type_from_message(message);

                    // TODO: checksum
                    if (strcmp(sender_type,"REQUEST_FORWARD") == 0){
						handle_request_forward_message(message);
						valid_packet = true;
					}
                    else {

                        int sender_id = get_tx_id_from_msg(message);
                        int receiver_id = get_rx_id_from_msg(message);
                        int checksum = get_checksum_from_msg(message);
                        int sender_type_int = get_int_type_from_msg(message);
                        bool checksum_correct = valid_message(sender_type_int, sender_id, receiver_id, checksum);
                        if (debug == true){
                            printf("Checksum: %d\n", checksum);
                        }
                        
                        if (checksum_correct==true){
                            if (debug == true){
                                printf("Sender Type: %s\n", sender_type);
                                printf("tx_id: %d\n", sender_id);
                                printf("rx_id: %d\n", receiver_id);
                            }
                        }
                        if (checksum_correct==true){
                            // Update local list
                            bool in_local_list = id_in_list(network_ids, sender_id);
                            if (in_local_list == false){
                                update_network_ids(sender_id, rssi);
                                broadcast_list_changed = true;
                                if (debug == true){
                                    printf("Update Local list - new id\n");
                                }
                            }
                            else{
                                update_rssi_list(sender_id, rssi);
                                if (debug == true){
                                    printf("Update Local list - new rssi\n");
                                }
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
					printf("\n");
					setIDLE();
					cc1200_cmd(SFRX);
					packet_len = 0;
					setRX();
                    if (debug == true){
                        printf("CLOCK - END: %d\n",clock()* 1000/CLOCKS_PER_SEC);
                    }
				}
			}
		}
	
		// check if local list changed?
        if (potential_forwarder_ids[0] != 0){
            choose_forwarder();
			printf("SELECTED FORWARDER\n");
		}
		else if ((broadcast_list_changed == true)&&(loop_counter%3==0)){
			printf("broadcast new list....\n");
			broadcast_list_changed = false;
			send_list_message(network_ids, num_nodes);
            if ((loop_counter%30)==0){
                printf("reset all lists\n");
                for (i = 0; i<6; ++i){
                    network_ids[i] = 0;
                    rssi_values[i] = 0;
                }
                network_ids[0] = id;
                rssi_values[0] = 127;
            }
		}   
        else if ((heartbeat_send == false)){
			send_message(0x03, id, id);
			printf("HEARTBEAT\n");
			heartbeat_send = true;
		}



	}

}