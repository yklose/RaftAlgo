/****************************************************************************/
/*                                                                          */
/* Structure of programs controlling CC1200                                 */
/*                                                                          */
/****************************************************************************/

#include "lib.h"
#include "variables.h"
#include "message_handling.h"
#include <stdio.h>
#include <SPIv1.h> 
#include "registers.h"  
#include <unistd.h>
#include <string.h>


#define LEN(x)  (sizeof(x) / sizeof((x)[0]))

void load_variables(void){
	// load needed extern variables	
	extern int packet_len;      
	extern int max_packet_len;  
	extern network_ids[];
	// initialize generated numbers
	extern int id;
	extern char string_id[];
	id  = generate_random_id(); 
	network_ids[0] = id;
	extern int state;
	state = set_state_open();
	// print ID
	printf("RANDOM ID: %d\n", id);
}


int main (void) {

	// load all variables
	load_variables();

	// first initialize
	initialize_spi();
	
	// write registers from file
	int cnt;
	int adr;
	int val;
	for (cnt=0; cnt<MAX_REG; cnt++) cc1200_reg_write(RegSettings[cnt].adr, RegSettings[cnt].val);
	for (cnt=0; cnt<MAX_EXT_REG; cnt++) cc1200_reg_write(ExtRegSettings[cnt].adr, ExtRegSettings[cnt].val);
	cc1200_reg_write(PKT_CFG0, 0x01);			
	cc1200_reg_write(PKT_LEN, max_packet_len);
	
	// set RX mode
	setRX();
	
	// initialize generated numbers
	cc1200_reg_write(RNDGEN, 0xFF); //activate random numbers
	cc1200_cmd(SNOP);
	int int_id1;
	cc1200_reg_read(RNDGEN, &int_id1);
	int int_id2;
	cc1200_reg_read(RNDGEN, &int_id2);
	sprintf(string_id, "%03d%03d", int_id1,int_id2);
	printf("ID: %s\n", string_id);
	
	// starting configurations
	extern int state;
	if (state_open(state)){
		printf("INITIAL STATE: OPEN\n\n");
	}

	// loop for incoming packet
	read_incoming_packet_loop();

	// loop for leader
	leader_loop();
	
	// shutdown SPI
	spi_shutdown();
	return 0;
}

