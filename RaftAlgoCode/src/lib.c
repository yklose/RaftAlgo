/****************************************************************************/
/*                                                                          */
/* Useful functions receiving/sending packets with CC1200                   */
/*                                                                          */
/****************************************************************************/
#include "lib.h"

#include <stdio.h>
#include <SPIv1.h> // necessary, otherwise CC1200 prototype are not available
#include <unistd.h>
#include <stdbool.h>

void rssi_valid(int rssi_add) {
        int val;
        cc1200_reg_read(rssi_add,&val);
        while ((val & 0x1) == 0) {
                 cc1200_reg_read(rssi_add, & val);
        }
}
int read_rssi1(int rssi_add) {
        int val;
        cc1200_reg_read(rssi_add,&val);
        return val;
}

void initialize_spi(){
        if(spi_init()){
    	        printf("ERROR: Initialization failed\n");
   		exit(-1);
 	}
        cc1200_cmd(SRES);
}

int generate_random_timeout(){
	srand(time(NULL));
	int max = 500;
	return rand() % max;
}

int generate_random_id(){
	srand(time(NULL));
	int min = 1000000;
	int max = 9999999;
	return (rand() % (max + 1 - min)) + min;
}

bool state_open(int state){
	if (state == 0x00){
		return true;
	}
	else{
		return false;
	}
}

bool state_follower(int state){ 
        if (state == 0x01){ 
                return true;
        }
        else{ 
                return false;
        }
}

bool state_proposer(int state){ 
        if (state == 0x02){ 
                return true;
        }
        else{ 
                return false;
        }
}

bool state_leader(int state){ 
        if (state == 0x03){ 
                return true;
        }
        else{ 
                return false;
        }
}

int set_state_open(){
	return 0x00;
}

int set_state_follower(){
	return 0x01;
}

int set_state_proposer(){
	return 0x02;
}

int set_state_leader(){
	//LEADER STATE

	//LEADER STATE
	return 0x03;
}

void setIDLE() {
    cc1200_cmd(SIDLE);
    cc1200_cmd(SNOP);
    while (get_status_cc1200() != IDLE) {
        cc1200_cmd(SNOP);
        cc1200_cmd(SIDLE);
    }
}

void setRX() {
    cc1200_cmd(SRX);
    cc1200_cmd(SNOP);
    while (get_status_cc1200() != RX) {
        cc1200_cmd(SNOP);
        cc1200_cmd(SRX);
    }
}

void setTX() {
    cc1200_cmd(STX);
    cc1200_cmd(SNOP);
    while (get_status_cc1200() != TX) {
        cc1200_cmd(SNOP);
        cc1200_cmd(STX);
    }
}

bool valid_message(int message_type, int tx_id, int rx_id, int checksum){
        int modulus = 999;
        int sum = message_type + tx_id + rx_id;
        return (sum%modulus == checksum);       
}

int compute_checksum(int message_type, int tx_id, int rx_id){
        int modulus = 999;
        int sum = message_type + tx_id + rx_id;
        return (sum%modulus);     
}       

void send_message(int message_type, int tx_id, int rx_id){

	//char msg[] = "HelloWorld0";
	char msg[20];
        int checksum = compute_checksum(message_type, tx_id, rx_id);
	sprintf(msg, "%d%d%d%d%d",message_type, tx_id, rx_id, checksum, 0x00); 
	printf("TransmitMessageString: %s\n", msg);
	setIDLE();
	cc1200_cmd(SFTX);
	cc1200_reg_write(0x3F, strlen(msg));
	int j;
	for (j=0; j<strlen(msg); ++j){
        	cc1200_cmd(SNOP);
        	cc1200_reg_write(0x3F, msg[j]);
        	cc1200_cmd(SNOP);
	}
	int numTx;
	cc1200_reg_read(0x2FD6, &numTx);
	setTX();
        while((get_status_cc1200()==TX) || (numTx != 0)){
                cc1200_cmd(SNOP);
                cc1200_reg_read(0x2FD6, & numTx);
        }
	printf("DONE TRANSMITTING\n\n");
	setIDLE();
}

int convert_char_to_int(char *msg){
	int msg_int;
        sscanf(msg, "%d", &msg_int);
        return msg_int;
}


int get_tx_id_from_msg(char *msg){
	int id_len = 7;
	char id[id_len];
	int i;
	for (i=0; i<id_len; ++i){
		id[i] = msg[i+1];
	}
	return convert_char_to_int(id);
}

int get_rx_id_from_msg(char *msg){
        int id_len = 7;
        char id[id_len];
        int i;
        for (i=0; i<id_len; ++i){
                id[i] = msg[i+1+id_len];
        }
        return convert_char_to_int(id);
}

int get_checksum_from_msg(char *msg){
        int id_len = 7;
	int checksum_len = 3;
	char checksum[checksum_len];
	int i;
	for (i=0; i<checksum_len; ++i){
		checksum[i] = msg[i+1+id_len+id_len];
	}
	return convert_char_to_int(checksum);
}

int get_int_type_from_msg(char *msg){
        char type[1];
        type[0] = msg[0];
	return convert_char_to_int(type);
}

int *get_broadcast_ids_from_msg(int *broadcast_network_ids, char *msg){
        int message_len = strlen(msg);
        int checksum_len = 3;
        int id_len = 7;
        int num_ids = (message_len-checksum_len-1)/id_len;
	//int output_ids[num_ids];
	//memset(output_ids, 0, sizeof output_ids);
        int i;
        for (i=0; i<num_ids;++i){
                int j;
                char id[id_len];
                for (j=0; j<id_len; ++j){
			int index = 1+j+i*id_len;
			printf("index: %d\n", index);
                        id[j] = msg[index];
                }
		int id_int = convert_char_to_int(id);
		broadcast_network_ids[i] = id_int;
                printf("Current ID: %d\n",broadcast_network_ids[i]);
        }
	return broadcast_network_ids;

}

void process_list_broadcast(int* global_network_ids, int len_global_network_ids, int* network_ids,int len_network_ids, int id){
        int i;
	for (i=0; i<(len_network_ids); ++i){
		printf("ID: %d\n", network_ids[i]);
                int j;
                bool found = false;
		for (j=0; j<(len_global_network_ids); ++j){
			if (network_ids[i]==global_network_ids[j]){
				found = true;
			}
		}
                if (found == false){
                        printf("Not in list...\n");
			send_message(6, id, network_ids[i]);
                }
                else{
                        printf("already in list \n");
                }
        }
}


char *get_type_from_message(char *msg){
	char *message;
	if (msg[0] == '0'){
		message = "PROPOSE";
	}
	else if (msg[0] == '1'){
                message = "ACCEPT_OK";
        }
	else if (msg[0] == '2'){
                message = "ACCEPT_NOT";
        }
	else if (msg[0] == '3'){
                message = "LEADER";
        }
	else if (msg[0] == '4'){
                message = "OK";
        }
        else if (msg[0] == '5'){
                message = "LIST_BROADCAST";
        }
        else if (msg[0] == '6'){
                message = "FORWARD_OK";
        }
        else if (msg[0] == '7'){
                message = "REQUEST_FORWARD";
        }
	else{
		message = "ERROR";
	}
	return message;
}

bool id_in_list(int *id_list, int id, int num_nodes){
        int node;
        for(node=0;node<num_nodes;++node){
                if(id == id_list[node]){
                        return true;
                }
        }
        return false;
}

int *update_RSSI_list(int *rssi_values, int *network_ids, int sender_id, int rssi_value, int num_nodes){
        int n;
        for (n=0; n<num_nodes;++n){
                if (network_ids[n]==sender_id){
                        rssi_values[n]=rssi_value;
                        break;
                }
        }
        return rssi_values;       
}


