/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/

#include "lib.h"
#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
//#include "test.h"
#include "ex2.h"  //Registers
#include <unistd.h>
#include <string.h>



int main (void) {

	int adr;
	int val;

  	// first initialize
  	if(spi_init()){
    		printf("ERROR: Initialization failed\n");
   		 return -1;
 	 }

	// reset CC1200
	cc1200_cmd(SRES);
	// CC1200 is now in idle mode, registers have their default values
	int cnt;
	for (cnt=0; cnt<MAX_REG; cnt++) cc1200_reg_write(RegSettings[cnt].adr, RegSettings[cnt].val);
	for (cnt=0; cnt<MAX_EXT_REG; cnt++) cc1200_reg_write(ExtRegSettings[cnt].adr, ExtRegSettings[cnt].val);

	// set random countdown
	int msec = 0;
	int timeout = 2000; //generate_random_timeout();
	printf("Random timeout: %d\n", timeout);
	// set RX mode
	int numRX;
	int packet_len = 0;
	int max_packet_len = 0x14;
	int fifo;
	char message[max_packet_len];
	cc1200_reg_write(PKT_CFG0, 0x01); 		// variable packet len
	cc1200_reg_write(PKT_LEN, max_packet_len);  	// maximum packet len = 10 CHANGE!
	setRX();
	// get random ID
	int id = generate_random_id();
	printf("RANDOM ID: %d\n\n", id);
	// set state
	int state = set_state_open();
	if (state_open(state)){
		printf("INITIAL STATE: OPEN\n");
	}
	// ACCEPT counter
	float accept_counter = 0;
	float accept_not_counter = 0;
	int num_nodes = 6;

	//Save IDs
	int proposer_id = 0;
	int leader_id = 0;
	// test message
	//send_message(0x01, id);

	//Received_ids
	int follower_ids[6] = {0};

	while(1){ //DAUERSCHLEIFE
		printf("RESET TIMER - STATE %x\n\n",state);
		// set RX
		setRX();
		// start clock
		clock_t starttime = clock();
		clock_t difference = clock()-starttime;
		msec = difference * 1000 / CLOCKS_PER_SEC;
//		printf("MSEC: %d\n", msec);
//		printf("TIMOUT: %d\n", timeout);
		//RX LOOP
		while (msec < timeout){
			// set up timer
			difference = clock() - starttime;
			msec = difference * 1000 / CLOCKS_PER_SEC;
			// read number of bytes
			cc1200_reg_read(NUM_RXBYTES, &numRX);
			// packet
			if(numRX>0){
				printf("----------- PACKET -----------\n");
        	        	if (packet_len == 0){
	                        	cc1200_reg_read(0x3F, &packet_len);
//           		             	printf("LEN: %x\n", packet_len);
                        		if (packet_len>max_packet_len){
                  		              	packet_len = max_packet_len;
                                		printf("Transmitted message is longer than max configured lengths\n");
                     
		  			}
                        		int k = 0;
                        		for (k=0; k<packet_len; ++k){
                                		cc1200_reg_read(0x3F, &fifo);
						message[k] = (char)fifo;
						// printing is somehow needed! DONT KNOW WHY!
						printf("READING: %c\n", message[k]);
                      			}
					message[k+1] = '\0';
					printf("\n");
					printf("ReceivedMessage: %s\n",message);
	
					// get message type
					int sender_id = get_tx_id_from_msg(message);
					int receiver_id = get_rx_id_from_msg(message);
                	                char *sender_type = get_type_from_message(message);
					printf("Sender Type: %s\n", sender_type);
					printf("tx_id: %d\n", sender_id);
					printf("rx_id: %d\n", receiver_id);

					if (strcmp(sender_type,"PROPOSE") == 0){
						//Received PROPOSE: send Accept ok when open else not accept
						printf("PROPOSE MESSAGE\n");
						if ((state_open(state))||(sender_id == proposer_id)){	// If OPEN or FOLLOWER of this proposer
							printf("SEND ACCEPT OK message\n");
							state = set_state_follower();
							send_message(0x01, id, sender_id);
							printf("Time: %d\n", msec);
							//save proposer ID
							printf("save proposer id\n");
							proposer_id = sender_id;
						}
						else{
							printf("SEND ACCEPT DECLINE message\n");
							send_message(0x02, id, sender_id);
                                        	        printf("Time: %d\n", msec);
							// send accept 0 message
						}
					}

					else if (strcmp(sender_type,"ACCEPT_OK") == 0){
						//Received ACCEPT OK: Proposers increase counter and maybe switch to LEADER
						printf("ACCEPT OK MESSAGE\n");
						// increase accept counter
						if  (state_proposer(state)){
							int node;
							bool found = false;
							for(node=0;node<num_nodes;++node){
								printf("node %d\n",node);
								if(sender_id == follower_ids[node]){
									found = true;
								}
							}
							printf("loop end\n");
							if (found == false){
								printf("INCREASE ACCEPT COUNTER\n");
								accept_counter = accept_counter + 1.0;//Nur wenn noch nicht in L$
								int n;
								for (n=0; n<num_nodes;++n){
									if (follower_ids[n]==0){
										follower_ids[n]=sender_id;
										break;
									}
								}
								printf("Anzahl der Follower: %d\n",accept_counter);
							}
							printf("partition of followers: %f\n",(accept_counter/num_nodes));
							//printf("partition of followers: %f",(float(accept_counter)/num_nodes));
							if ((accept_counter/num_nodes)>0.5) { //MAJORITY?
								printf("I AM LEADER\n");
								//CHANGE TO LEADER
								state = set_state_leader();
								//send Leader msg
								accept_counter = 0;
								accept_not_counter = 0;
							}
						}
					}

					else if (strcmp(sender_type,"ACCEPT_NOT") == 0){
						//Received ACCEPT NOT: Proposers increase counter and maybe switch to OPEN 
                                	      	printf("ACCEPT NOT MESSAGE\n");
						// increase decline counter
						if  (state_proposer(state)){
                                                	printf("INCREASE ACCEPT NOT COUNTER\n");
                                                	accept_not_counter += 1;
							if ((num_nodes/accept_not_counter)>=2) { //MINORITY?
                                                        	//CHANGE TO OPEN
								printf("Clear Minority - SET OPEN\n");
                                                        	state = set_state_open();
								accept_not_counter = 0;
								accept_counter = 0;
                                                	}
                        	                }
                	               	}

					else if (strcmp(sender_type,"LEADER") == 0){
						//Received LEADER: All change to follower
                                        	printf("Send follower message\n");
						state = set_state_follower();
						// send ok message
						send_message(0x04, id, sender_id);
						//save leader id
						leader_id = sender_id;
                                	}

					else if (strcmp(sender_type,"OK") == 0){
						// Received OK: Leaders count listeners
        	                                printf("OK MESSAGE\n");
						if  (state_leader(state)){
							//ADD TO LIST OF LISTENERS
		  					printf("Listeners +1");
						}
						// do nothing (see how many are listening)
                	                }

					// go in IDLE mode to Reset FIFO
					printf("\n\n");
					setIDLE();
					cc1200_cmd(SFRX);
					packet_len = 0;
					setRX();

					// Reset Timer
                    printf("RESET TIMER\n");
					starttime = clock();
                    // msec = timeout;
					}	
       			}
		}
		//TIMER ABGELAUFEN
		if (state_leader(state)==false){ 
		printf("SEND PROPOSE\n");
		state = set_state_proposer();
		send_message(0x00, id, id);
		printf("SEND SUCCESS\n");
		}
		accept_counter = 0;
		accept_not_counter = 0; 
	} //END while(1) LOOP


	// go to receive mode if others send message

	// wait in loop for countdown 

	// shutdown SPI
	spi_shutdown();
	return 0;
}

