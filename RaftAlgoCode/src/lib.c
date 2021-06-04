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
	sprintf(msg, "%d%d%d%d%d",message_type, tx_id, rx_id, checksum, 0x00); // TODO: add checksum (1 bit)
	printf("TransmitMessageString: %s\n", msg);
	setIDLE();
	cc1200_cmd(SFTX);
	cc1200_reg_write(0x3F, strlen(msg));
	int j;
	for (j=0; j<strlen(msg); ++j){
        	printf("Transmit: %c\n", msg[j]);
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
        int id_len = 3;
	int checksum_len = 3;
	char checksum[checksum_len];
	int i;
	for (i=0; i<checksum_len; ++i){
		checksum[i] = msg[i+1+id_len+id_len];
	}
	return convert_char_to_int(checksum);
}

int get_int_type_from_msg(char *msg){
        char type[1] = msg[0];
	return convert_char_to_int(type);
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
	else{
		message = "ERROR";
	}
	return message;
}

bool id_in_list(int *follower_ids, int id, int num_nodes){
        int node;
        for(node=0;node<num_nodes;++node){
                if(id == follower_ids[node]){
                        return true;
                }
        }
        return false;
}
