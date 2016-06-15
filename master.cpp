//
//  master.cpp
//  
//
//  Created by Anupama Kumar on 12/17/15.
//
//

// MASTER FILE //

#include "mbed.h"
#include "MRF24J40.h"
#include <string>

// RF tranceiver to link with handheld.
MRF24J40 mrf(p11, p12, p13, p14, p21);

// LEDs you can treat these as variables (led2 = 1 will turn led2 on!)
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

// Timer
Timer timer;

// Serial port for showing RX data.
Serial pc(USBTX, USBRX);

// Used for sending and receiving
char txBuffer[128];
char rxBuffer[128];
int rxLen;

//***************** Do not change these methods (please) *****************//

/**
 * Receive data from the MRF24J40.
 *
 * @param data A pointer to a char array to hold the data
 * @param maxLength The max amount of data to read.
 */
int rf_receive(char *data, uint8_t maxLength)
{
    uint8_t len = mrf.Receive((uint8_t *)data, maxLength);
    uint8_t header[8]= {1, 8, 0, 0xA1, 0xB2, 0xC3, 0xD4, 0x00};
    
    if(len > 10) {
        //Remove the header and footer of the message
        for(uint8_t i = 0; i < len-2; i++) {
            if(i<8) {
                //Make sure our header is valid first
                if(data[i] != header[i])
                    return 0;
            } else {
                data[i-8] = data[i];
            }
        }
        
        //pc.printf("Received: %s length:%d\r\n", data, ((int)len)-10);
    }
    return ((int)len)-10;
}

/**
 * Send data to another MRF24J40.
 *
 * @param data The string to send
 * @param maxLength The length of the data to send.
 *                  If you are sending a null-terminated string you can pass strlen(data)+1
 */
void rf_send(char *data, uint8_t len)
{
    //We need to prepend the message with a valid ZigBee header
    uint8_t header[8]= {1, 8, 0, 0xA1, 0xB2, 0xC3, 0xD4, 0x00};
    uint8_t *send_buf = (uint8_t *) malloc( sizeof(uint8_t) * (len+8) );
    
    for(uint8_t i = 0; i < len+8; i++) {
        //prepend the 8-byte header
        send_buf[i] = (i<8) ? header[i] : data[i-8];
    }
    //pc.printf("Sent: %s\r\n", send_buf+8);
    
    mrf.Send(send_buf, len+8);
    free(send_buf);
}


//***************** You can start coding here *****************//

void checkNode2Alive() {
    while(1) {
        rxLen = rf_receive(rxBuffer, 128);
        if (rxLen>0){
            pc.printf("Node 2 is alive\r\n");
            break;
        }
    }
}

void checkNode1Alive() {
    while(1) {
        rxLen = rf_receive(rxBuffer, 128);
        if (rxLen>0){
            pc.printf("Node 1 is alive\r\n");
            break;
        }
    }
}

void sendNode2AliveCheck() {
    strcpy(txBuffer, "check_2");
    rf_send(txBuffer, strlen(txBuffer)+1);
    pc.printf("Sent: %s\r\n", txBuffer);
    
}

void sendNode1AliveCheck() {
    strcpy(txBuffer, "check_1");
    rf_send(txBuffer, strlen(txBuffer)+1);
    pc.printf("Sent: %s\r\n", txBuffer);
}

void checkNodesAlive() {
    
    sendNode1AliveCheck();
    wait_ms(500);
    checkNode1Alive();
    
    sendNode2AliveCheck();
    wait_ms(500);
    checkNode2Alive();
}


int main (void)
{
    uint8_t channel = 11;
    
    //Set the Channel. 0 is default, 15 is max
    mrf.SetChannel(channel);
    
    int action;
    int round_counter;
    int score_counter;
    
    int best_score=0;
    
    int level;
    
    int number_of_nodes=2;
    
    char input;
    
    //Start the timer
    timer.start();
    
    checkNodesAlive();
    
    while (true){
        pc.printf("Enter Level (1-3) \r\n");
        input=pc.getc();
        switch (input)
        {
            case '1':
                level=1;
                break;
            case '2':
                level=2;
                break;
            case '3':
                level=3;
                break;
        }
        
        while (true)
        {
            pc.printf("Press 's' to start \r\n");
            input=pc.getc();
            if (input=='s')
                break;
        }
        while(true) {
            
            pc.printf("start!");
            round_counter=0;
            score_counter=0;
            
            while(round_counter<10){
                action = rand()%(number_of_nodes+1);
                if(action==0){
                    wait_ms(2000/level);
                }
                else if (action==1){
                    timer.reset();
                    
                    strcpy(txBuffer, "activate_1");
                    rf_send(txBuffer, strlen(txBuffer)+1);
                    
                    pc.printf("Sent: %s\r\n", txBuffer);
                    while(true){
                        rxLen = rf_receive(rxBuffer, 128);
                        
                        if (timer.read_ms()>((1000+(500*(10-round_counter)))/level)){
                            strcpy(txBuffer, "time_out_1");
                            rf_send(txBuffer, strlen(txBuffer)+1);
                            pc.printf("Sent: %s\r\n", txBuffer);
                            score_counter+=((500*(10-round_counter))/level)+2000;
                            break;
                        }
                        else if (rxLen>0){
                            pc.printf("Received: %s\r\n", rxBuffer);
                            if(!strcmp("finished_1", rxBuffer)) {
                                score_counter+=timer.read_ms();
                                break;
                            }
                        }
                    }
                    round_counter++;
                }
                else if (action==2){
                    timer.reset();
                    strcpy(txBuffer, "activate_2");
                    rf_send(txBuffer, strlen(txBuffer)+1);
                    pc.printf("Sent: %s\r\n", txBuffer);
                    while(true){
                        rxLen = rf_receive(rxBuffer, 128);
                        if (timer.read_ms()>((1000+(500*(10-round_counter)))/level)){
                            strcpy(txBuffer, "time_out_2");
                            rf_send(txBuffer, strlen(txBuffer)+1);
                            pc.printf("Sent: %s\r\n", txBuffer);
                            score_counter+=((500*(10-round_counter))/level)+2000;
                            break;
                        }
                        else if (rxLen>0){
                            pc.printf("Received: %s\r\n", rxBuffer);
                            if(!strcmp("finished_2", rxBuffer)) {
                                score_counter+=timer.read_ms();
                                break;
                            }
                        }
                    }
                    round_counter++;
                }
            }
            pc.printf("Score: %d \r\n", score_counter);
            if (score_counter<best_score){
                best_score=score_counter;
            }
            while (true){
                pc.printf("New game? (y/n)\r\n");
                input=pc.getc();
                if (input=='y'|| input=='n'){
                    break;
                }
            }
            if (input=='n'){
                pc.printf("Best Score %d", best_score);
                break;
            }
            
        }
    }
}