/****************************************************************************/
/*                                                                          */
/* Useful functions receiving/sending packets with CC1200                   */
/*                                                                          */
/****************************************************************************/


/****************************************************************************/
/*                                                                          */
/* TODO:
- checksum in list broadcast, problem: aufnfüllen von nullen
- ids mit hardware zwei ints concat
*/
/****************************************************************************/


#include "lib.h"
#include <stdio.h>
#include <SPIv1.h> 
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

/*
int update_msec(int starttime){
        clock_t difference = clock()-starttime;
	int msec = difference * 1000 / CLOCKS_PER_SEC;
        return msec;

}
*/
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

bool valid_request_message(int forwarder_id, int tx_id, int rssi, int checksum){
        int modulus = 999;
        int sum = 0x07 + forwarder_id + tx_id + rssi;
        return (sum%modulus == checksum);       
}

bool valid_list_message(int checksum){
        extern int network_ids[];       
        int num_ids = num_valid_ids_in_list(network_ids);
        int modulus = 999;
        int sum = 0x05;
        int i;
        for (i=0; i<num_ids; ++i) {
                sum = sum + network_ids[i];
        }
        return (sum%modulus == checksum);       
}

int num_valid_ids_in_list(int *array){
        int i;
        int num_valid_nodes = 0;
        //extern int num_nodes; // testin purpose 5
        for (i=0; i<5; ++i) {
                if (array[i] != 0) {
                        ++num_valid_nodes;
                }
        }
        return num_valid_nodes;
}

int compute_checksum(int message_type, int tx_id, int rx_id){
        int modulus = 999;
        int sum = message_type + tx_id + rx_id;
        return (sum%modulus);     
}    

int compute_list_checksum(int message_type, int *network_ids, int num_ids){
        int modulus = 999;
        int sum = message_type;
        int i;
        for (i=0; i<num_ids; ++i) {
                sum = sum + network_ids[i];
                printf("ADD %d to list_checksum\n", network_ids[i]);
        }
        return (sum%modulus);     
}  

int compute_request_checksum(int forwarder_id, int tx_id, int rssi){
        int modulus = 999;
        int sum = 0x07 + forwarder_id + tx_id + rssi;
        return (sum%modulus);     
}  


void send_request_message(int forwarder_id, int tx_id, int rssi){
        char msg[20];
        int checksum = compute_request_checksum(forwarder_id, tx_id, rssi);
        if (rssi > 99){
                if (checksum > 99){
                        sprintf(msg, "%d%d%d%d%d%d",0x07, forwarder_id, tx_id, rssi ,checksum, 0x00);
                }
                else if (checksum > 9){
                        sprintf(msg, "%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, rssi ,0x00, checksum, 0x00);
                }
                else{
                        sprintf(msg, "%d%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, rssi ,0x00, 0x00, checksum, 0x00);
                }
        }
        else if (rssi > 9){
                if (checksum > 99){
                        sprintf(msg, "%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, 0x00 ,rssi ,checksum, 0x00); 
                }
                else if (checksum > 9){
                        sprintf(msg, "%d%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, 0x00 ,rssi , 0x00, checksum, 0x00); 
                }
                else{
                        sprintf(msg, "%d%d%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, 0x00 ,rssi ,0x00, 0x00, checksum, 0x00); 
                }
        }
        else {
                if (checksum > 99){
                        sprintf(msg, "%d%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, 0x00 , 0x00 ,rssi ,checksum, 0x00); 
                }
                else if (checksum > 9){
                        sprintf(msg, "%d%d%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, 0x00 , 0x00 ,rssi ,0x00, checksum, 0x00); 
                }
                else{
                        sprintf(msg, "%d%d%d%d%d%d%d%d%d%d",0x07, forwarder_id, tx_id, 0x00 , 0x00 ,rssi , 0x00, 0x00, checksum, 0x00); 
                }
        }
        // print message string
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

int time_to_wait(){
        extern int num_nodes;
        cc1200_reg_write(0x2F80, 0xFF); //activate random numbers
	cc1200_cmd(SNOP);
        int rnd_int;
	cc1200_reg_read(0x2F80, &rnd_int);
	rssi_valid(0x2F72);  //RSSI0 = 0x72
	int rssi = read_rssi1(0x2F71);
	return (rnd_int^rssi)%10*(num_nodes);
}

void send_message(int message_type, int tx_id, int rx_id){
       int time = time_to_wait();
        printf("time_to_wait: %d\n", time);
        sleep(time/1000);
	//char msg[] = "HelloWorld0";
	char msg[20];
        int checksum = compute_checksum(message_type, tx_id, rx_id);

        sprintf(msg, "%d%d%d%03d%d",message_type, tx_id, rx_id, checksum, 0x00);
        /*
        if (checksum > 99){
                sprintf(msg, "%d%d%d%d%d",message_type, tx_id, rx_id, checksum, 0x00);
        }
        else if (checksum > 9){
                sprintf(msg, "%d%d%d%d%d%d",message_type, tx_id, rx_id, 0x00, checksum, 0x00);
        }
        else{
                sprintf(msg, "%d%d%d%d%d%d%d",message_type, tx_id, rx_id, 0x00, 0x00, checksum, 0x00);
        } 
        */
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

void send_list_message(int *network_ids, int num_nodes){
        int i;
        int num_ids_to_send = 0;
        for (i=0; i<5; ++i) {  //TESTING 5=num_nodes
                if (network_ids[i] != 0) {
                        ++num_ids_to_send;
                        printf("IDs: %d\n", network_ids[i]);
                }
        }

	//char msg[] = "HelloWorld0";
        int message_type = 0x05;
        int msg_len = (num_ids_to_send*7)+1+4+2;

        char msg[msg_len];
	memset(msg, 0, sizeof msg);

	//char msg[40]; // char msg[msg_len];
        int checksum = compute_list_checksum(message_type, network_ids, num_ids_to_send);

        char* msg_type = "5";
        strcpy( msg, msg_type );

        int j;
        for (j=0; j<num_ids_to_send+2; ++j) {
                
                if (j==num_ids_to_send){
                        char buffer[3];
                        sprintf(buffer, "%03d", checksum);
                        strcat( msg, buffer );
                }
                else if (j==num_ids_to_send+1){
                        char buffer[1];
                        sprintf(buffer, "%d", 0x00);
                        strcat( msg, buffer );
                }
                else {
                        char buffer[7];
                        sprintf(buffer, "%d", network_ids[j]);
                        strcat( msg, buffer );
                }
                
        } 	
	printf("TransmitMessageString: %s\n", msg);
	setIDLE();
	cc1200_cmd(SFTX);
	cc1200_reg_write(0x3F, strlen(msg));
	int k;
	for (k=0; k<strlen(msg); ++k){
        	cc1200_cmd(SNOP);
        	cc1200_reg_write(0x3F, msg[k]);
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
        int message_len = strlen(msg);
	int i;
	for (i=0; i<checksum_len; ++i){
		checksum[i] = msg[message_len-checksum_len-1+i];
	}
	return convert_char_to_int(checksum);
}

int get_int_type_from_msg(char *msg){
        char type[1];
        type[0] = msg[0];
	return convert_char_to_int(type);
}

void get_broadcast_ids_from_msg(char *msg, int *broadcast_network_ids, int len_broadcast_network_ids, int* network_ids,int len_network_ids, int id){
        extern int rssi_values[];
        int id_len = 7;
        /*
        int message_len = strlen(msg);
        int checksum_len = 3;
        int id_len = 7;
        int num_ids = (message_len-checksum_len-1)/id_len;
        */
	//int output_ids[num_ids];
	//memset(output_ids, 0, sizeof output_ids);
        
        int i;
        printf("len_broadcast_network_ids: %d\n", len_broadcast_network_ids);
        for (i=0; i<len_broadcast_network_ids;++i){
                
                int j;
                char id[id_len];
                for (j=0; j<id_len; ++j){
			int index = 1+j+i*id_len;
			//printf("index: %d\n", index);
                        id[j] = msg[index];
                }
		int id_int = convert_char_to_int(id);
		broadcast_network_ids[i] = id_int;
                printf("Current ID: %d\n",broadcast_network_ids[i]);
        }
	// return broadcast_network_ids;

        network_ids[1] = 1234567; //JUST FOR TESTING!

        int k;
	for (k=0; k<(len_network_ids); ++k){
		 printf("ID: %d", network_ids[k]);
                if (network_ids[k]!=0){
                       
                        int l;
                        bool found = false;
                        for (l=0; l<(len_broadcast_network_ids); ++l){
                                if ((network_ids[k]==broadcast_network_ids[l])){
                                        found = true;
                                }
                        }
                        if (found == false){
                                printf(" (Not in list. SEND... with RSSI: %d )\n", rssi_values[k]);
                                //send_message(0x07, id, network_ids[k]);
                                send_request_message(network_ids[k], id, rssi_values[k]);
                        }
                        else{
                                printf(" (already in list) \n");
                        }
                }
                
        }

}

void process_list_broadcast(void){
        extern int global_network_ids[];
        extern int network_ids[];
        extern int rssi_values[];
        extern int id;
        int len_global_network_ids = 2;
        int len_network_ids = 2;
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
                        printf("Not in list. SEND... with RSSI: %d\n", rssi_values[i]);
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

bool id_in_list(int *id_list, int id){
        extern int num_nodes;
        int node;
        for(node=0;node<5;++node){ //TESTING 5=num_nodes
                if(id == id_list[node]){
                        return true;
                }
        }
        return false;
}

/*
void update_rssi_list(int sender_id, int rssi){
        // use global lists
        extern int num_nodes;
        extern int network_ids[];
        extern int rssi_values[];
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
        // use global lists
        extern int num_nodes;
        extern int network_ids[];
        extern int rssi_values[];
        // add sender_id to network_ids
        int n=0;
        for (n=0; n<num_nodes;++n){
                if (network_ids[n]==0){
                        network_ids[n]=sender_id;
                        rssi_values[n]=rssi;
                        break;
                }
        }
}

*/
