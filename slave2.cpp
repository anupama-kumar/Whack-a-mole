//
//  slave2.cpp
//  
//
//  Created by Anupama Kumar on 12/17/15.
//
//
// SLAVE(2) //

#include "mbed.h"
#include "MRF24J40.h"
#include <string>

// RF tranceiver to link with handheld.
MRF24J40 mrf(p11, p12, p13, p14, p21);
DigitalIn light_sensor(p20);
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

bool initCheckFlag = true;

void tellMasterButtonPushed() {
    strcpy(txBuffer, "finished_2");
    rf_send(txBuffer, strlen(txBuffer)+1);
    pc.printf("Sent: %s\r\n", txBuffer);
}

bool isCalledByMaster() {
    rxLen = rf_receive(rxBuffer, 128);
    if (rxLen>0){
        pc.printf("Received: %s\r\n", rxBuffer);
        if(!strcmp("activate_2", rxBuffer)) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


void initialCheck() {
    while(1) {
        rxLen = rf_receive(rxBuffer, 128);
        if (rxLen>0 && !strcmp("check_2", rxBuffer)){
            pc.printf("Received: %s\r\n", rxBuffer);
            
            strcpy(txBuffer, "confirm_2");
            rf_send(txBuffer, strlen(txBuffer)+1);
            pc.printf("Sent: %s\r\n", txBuffer);
            
            initCheckFlag = false;
            
            led4 = 1;
            break;
        }
    }
}

bool checkIfTimeOut() {
    rxLen = rf_receive(rxBuffer, 128);
    if (rxLen>0 && !strcmp("time_out_2", rxBuffer)){
        return true;
    } else {
        return false;
    }
}

void waitForButtonPressState() {
    bool is_time_out = false;
    while(1) {
        led1 = 1;
        if(light_sensor == 1) {
            tellMasterButtonPushed();
            break;
        } else {
            is_time_out = checkIfTimeOut();
            if (is_time_out) {
                break;
            }
        }
    }
}


int main (void)
{
    uint8_t channel = 11;
    mrf.SetChannel(channel);
    
    bool isCalled = false;
    
    
    while(1) {
        if (initCheckFlag) {
            initialCheck();
        }
        isCalled = isCalledByMaster();
        if (isCalled) {
            waitForButtonPressState();
        } else {
            led1 = 0;
        }
    }
    
}
