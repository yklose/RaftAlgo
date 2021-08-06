#define LEN(x)  (sizeof(x) / sizeof((x)[0]))

/*
void choose_fowarder(int *request_ids, int *forwarder_ids, int *forwarder_rssis){
    // check for duplicates in forwarder_ids list, send FORWARD_OK message to chosen forwarder
    
    bool already_send[6] = {false};
    int curr_forwarder;
    int i;
    for(i = 0; i < num_nodes - 1; i++){
        int highest_rssi=0;
        int winner_node;
        int forward_node;
        
        //chose current node to choose a forwarder for
        if (already_send[i] == false){ //TRUE or FALSE
            curr_forwarder = forwarder_ids[i];
            int j;
            for(j = i; j < num_nodes - 1; j++){
                // forwarder duplicate
                if (forwarder_ids[j] == curr_forwarder){
                    //if rssi is highest update forwarder
                    if (forwarder_rssis[j]>highest_rssi){
                        highest_rssi = forwarder_rssis[j];
                        winner_node  = request_ids[j];
                        forward_node = forwarder_ids[j];
                    }
                    already_send[j] = true;
                }
            }
            //SEND: FOWARD_OK to winner_node 

            send_message(6, winner_node, forward_node);
            update_network_ids(winner_node,highest_rssi);
        }
    } 
}
*/
