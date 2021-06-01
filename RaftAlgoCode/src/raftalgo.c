/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/

#include "lib.h"
#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
#include "registers.h"  //Registers
#include <unistd.h>
#include <string.h>

int main (void) {

	// set default values
	int adr;
	int val;
	int numRX = 0;					// num of RX bytes in fifo
	int packet_len = 0;				// length of the received packet
	int max_packet_len = 0x14;		// maximum packet length allowed
	int fifo = 0;					// variable for reading fifo
	char message[max_packet_len];	// message string 

	// first initialize
	initialize_spi()
	
	// write registers
	int cnt;
	for (cnt=0; cnt<MAX_REG; cnt++) cc1200_reg_write(RegSettings[cnt].adr, RegSettings[cnt].val);
	for (cnt=0; cnt<MAX_EXT_REG; cnt++) cc1200_reg_write(ExtRegSettings[cnt].adr, ExtRegSettings[cnt].val);

	// generate random ID
	int id = generate_random_id();
	printf("RANDOM ID: %d\n\n", id);

	// generate random countdown
	int msec = 0;
	int timeout = 2000; //generate_random_timeout();
	printf("Random timeout: %d\n", timeout);

	// set RX mode
	cc1200_reg_write(PKT_CFG0, 0x01);				// variable packet len
	cc1200_reg_write(PKT_LEN, max_packet_len);		// maximum packet len = 10 CHANGE!
	setRX();
	
	// set state
	int state = set_state_open();
	if (state_open(state)){
		printf("INITIAL STATE: OPEN\n");
	}

	// ACCEPT counter 
	float accept_counter = 0;
	float accept_not_counter = 0;
	int num_nodes = 6;

	// Save IDs
	int proposer_id = 0;
	int leader_id = 0;

	// Received_ids
	int follower_ids[6] = {0};
	follower_ids[0] = id;

	// Dauerschleife
	while(1){ 
		// print current state
		printf("Initialize Loop - STATE %x\n\n",state);

		// set RX
		setRX();

		// start clock
		clock_t starttime = clock();
		clock_t difference = clock()-starttime;
		msec = difference * 1000 / CLOCKS_PER_SEC;

		// RX loop
		while (msec < timeout){

			// set up timer
			difference = clock() - starttime;
			msec = difference * 1000 / CLOCKS_PER_SEC;

			// read number of bytes in fifo
			cc1200_reg_read(NUM_RXBYTES, &numRX);

			// if there is a packet detected
			if(numRX>0){
				printf("----------- PACKET detected -----------\n");

				if (packet_len == 0){ // NOTE: why do we need to check packet len?
					cc1200_reg_read(0x3F, &packet_len);
					// check if message is longer than expected
					if (packet_len>max_packet_len){
						packet_len = max_packet_len;
						printf("Transmitted message is longer than max configured lengths\n");
					}
					// read the message 
					int k = 0;
					for (k=0; k<packet_len; ++k){
						cc1200_reg_read(0x3F, &fifo);
						message[k] = (char)fifo;
						printf("READING: %c\n", message[k]);
					}
					message[k+1] = '\0';
					printf("\nReceivedMessage: %s\n",message);
	
					// get message informations
					int sender_id = get_tx_id_from_msg(message);
					int receiver_id = get_rx_id_from_msg(message);
					char *sender_type = get_type_from_message(message);
					printf("Sender Type: %s\n", sender_type);
					printf("tx_id: %d\n", sender_id);
					printf("rx_id: %d\n", receiver_id);

					// evaluate message types
					if (strcmp(sender_type,"PROPOSE") == 0){
						// Received PROPOSE: send Accept ok when open or follower (of this proposer), else not accept
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

					else if (strcmp(sender_type,"ACCEPT_OK") == 0){
						// Received ACCEPT OK: Proposers increase counter and maybe switch to LEADER
						printf("ACCEPT OK MESSAGE\n");
						if (state_proposer(state)){
							bool found = id_in_list(follower_ids, sender_id, num_nodes);
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
								printf("Anzahl ACCEPT Nachrichten: %d\n",accept_counter);
							}
							printf("partition of followers: %f\n",(accept_counter/num_nodes));
							// check if majority is reached
							if ((accept_counter/num_nodes)>0.5) { 
								printf("Clear Majority - SET LEADER\n");
								state = set_state_leader();
								// send Leader msg (NOTE: outside the loop)
								// reset old counter (not needed?)
								accept_counter = 0;
								accept_not_counter = 0;
							}
						}
					}

					else if (strcmp(sender_type,"ACCEPT_NOT") == 0){
						// Received ACCEPT NOT: Proposers increase counter and maybe switch to OPEN 
						printf("ACCEPT NOT MESSAGE\n");
						// increase decline counter
						if (state_proposer(state)){
							bool found = id_in_list(follower_ids, sender_id, num_nodes);
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
								printf("Anzahl ACCEPT NOT Nachrichten: %d\n",accept_not_counter);
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

					else if (strcmp(sender_type,"LEADER") == 0){
						// Received LEADER: All change to follower
						printf("LEADER MESSAGE\n");
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
							// TODO: ADD TO LIST OF LISTENERS
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
					// TODO: change timeout to random
				}	
			}
			printf("---------------------------------------\n");
		}
		// TIMER ABGELAUFEN
		if (state_leader(state)==false){ 
			printf("SEND PROPOSE\n");
			state = set_state_proposer();
			send_message(0x00, id, id);
		}
		// DEBUG deactivate! Otherwise keep!
		//accept_counter = 0;
		//accept_not_counter = 0; 
	}

	// shutdown SPI
	spi_shutdown();
	return 0;
}

