/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/

#include "lib.h"
#include "variables.h"
#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
#include "registers.h"  //Registers
#include <unistd.h>
#include <string.h>

#define LEN(x)  (sizeof(x) / sizeof((x)[0]))


void load_variables(void){
	extern int num_nodes;
	extern int numRX;		
	extern int packet_len;      
	extern int max_packet_len;  
	extern int fifo;           
	extern char message[];	
	
	//extern int id;            
	extern int proposer_id; 
	extern int leader_id;    
	extern int follower_ids[];
	extern int network_ids[];
	extern int rssi_values[];
	extern int global_network_ids[];
	extern int forwarder_ids[];
	extern int msec;
	extern int timeout;
	extern float accept_counter;
	extern float accept_not_counter;
	extern bool valid_packet;
	//extern int state;
}


int main (void) {

	// load all variables
	load_variables();

	int id  = generate_random_id();     // random ID of node
	int state = set_state_open();
	pass_id(id, state);

	// first initialize
	initialize_spi();
	
	// write registers
	int cnt;
	int adr;
	int val;
	for (cnt=0; cnt<MAX_REG; cnt++) cc1200_reg_write(RegSettings[cnt].adr, RegSettings[cnt].val);
	for (cnt=0; cnt<MAX_EXT_REG; cnt++) cc1200_reg_write(ExtRegSettings[cnt].adr, ExtRegSettings[cnt].val);
	cc1200_reg_write(PKT_CFG0, 0x01);			
	cc1200_reg_write(PKT_LEN, max_packet_len);

	// Print ID and random timeout
	printf("RANDOM ID: %d\n\n", id);
	printf("Random timeout: %d\n", timeout);

	// set RX mode
	setRX();
	
	// print state
	if (state_open(state)){
		printf("INITIAL STATE: OPEN\n");
	}

	
	// TESTING
	char testmsg[18] = "511111112222222333";
	int *test = get_broadcast_ids_from_msg(global_network_ids, testmsg);
	printf("Liste: %d\n",test[1]);
	int list1[3] = {1, 2, 3};
	int list2[3] = {2, 3, 4};
	process_list_broadcast(list1, LEN(list1), list2, LEN(list2), id);
	printf("testing\n");
	tester(1,40);
	print_values();
	tester(2,30);
	print_values();

	// loop for incoming packet
	read_incoming_packet_loop();

	/*
	int loop_counter = 0;
	// Leader Loop
	while (true){
		// print current state
		printf("Initialize Leader Loop \n\n");

		// set RX
		setRX();

		// start clock
		clock_t starttime = clock();
		clock_t difference = clock()-starttime;
		msec = difference * 1000 / CLOCKS_PER_SEC;
		int timeout = 2000; //generate_random_timeout();
		valid_packet = false;
		bool heartbeat_send = false;
		bool broadcast_list_changed = false;
		loop_counter++;
		// RX loop
		while (msec < timeout){
			// set up timer
			difference = clock() - starttime;
			msec = difference * 1000 / CLOCKS_PER_SEC;

			// read number of bytes in fifo
			cc1200_reg_read(NUM_RXBYTES, &numRX);

			// if there is a packet detected and you are not the leader!
			if(numRX>0){
				printf("----------- PACKET detected -----------\n");
				rssi_valid(RSSI0);
				int rssi = read_rssi1(RSSI1);
				printf("RSSI: %d\n", rssi);


				if (packet_len == 0){ // NOTE: why do we need to check packet len?
					cc1200_reg_read(0x3F, &packet_len);
					// check if message is longer than expected
					if (packet_len>max_packet_len){
						packet_len = max_packet_len;
						printf("Transmitted message is longer than max configured lengths\n");
					}
					// read the message TODO: abbruchbedingung! Falls packet kürzer
					int k = 0;
					while(k<packet_len){
						cc1200_reg_read(NUM_RXBYTES,&numRX);
						if (numRX>0){
							cc1200_reg_read(0x3F, &fifo);
							message[k] = (char)fifo;
							k = k + 1;
						}
					}
					message[k+1] = '\0';
					printf("\nReceivedMessage: %s\n",message);
	
					// get message informations
					int sender_id = get_tx_id_from_msg(message);
					int receiver_id = get_rx_id_from_msg(message);
					int checksum = get_checksum_from_msg(message);
					char *sender_type = get_type_from_message(message);
					int sender_type_int = get_int_type_from_msg(message);
					bool checksum_correct = valid_message(sender_type_int, sender_id, receiver_id, checksum);
					if (checksum_correct==true){
						printf("Sender Type: %s\n", sender_type);
						printf("tx_id: %d\n", sender_id);
						printf("rx_id: %d\n", receiver_id);
					}
					if (checksum_correct==true){
						// Update local list // TODO: counter if lost connection!
						bool in_local_list = id_in_list(network_ids, sender_id, num_nodes);
						if (in_local_list == false){
							broadcast_list_changed = true;
							// add sender_id to network_ids
							int n=0;
							for (n=0; n<num_nodes;++n){
								if (network_ids[n]==0){
									network_ids[n]=sender_id;
									rssi_values[n]=rssi;
									break;
								}
							}
							printf("Update Local list - new id");
						}
						else{
							int *rssi_values_new = update_RSSI_list(rssi_values, network_ids, sender_id, rssi, num_nodes);
							printf("Update Local list - new RSSI\n");
						}
						printf("done update local list\n");
						// evaluate message types
						if (strcmp(sender_type,"REQUEST_FORWARD") == 0){
							printf("REQUEST FORWARD MESSAGE\n");
							valid_packet = true;
						}
						if (strcmp(sender_type,"OK") == 0){
							printf("OK MESSAGE\n");
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
					// random timeout
					//int timeout = 2000; //generate_random_timeout();
					//starttime = clock();
				}
			}
		}
		if ((heartbeat_send == false)){
			send_message(0x03, id, id);
			heartbeat_send = true;
		}

		// check if local list changed?
		if ((broadcast_list_changed == true)&&(loop_counter%3==0)){
			printf("broadcast new list....\n");
			broadcast_list_changed = false;
		}

	}
	*/

	// shutdown SPI
	spi_shutdown();
	return 0;
}

