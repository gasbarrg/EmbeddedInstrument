    /*
 * 7Seg_Library.c
 *
 *  Created on: Dec 4, 2022
 *      Author: gasbarrg
 */
#include <Seg_Library.h>
#include <SysTick_Library12MHZ.h>
#include "Encoder_Library.h"

void sendCommand(unsigned char adress, unsigned char data){
    //Wait for Ready
    while(!(EUSCI_B3->IFG & 2));
    //Hold CS low during 2 transmits
    P10->OUT  |=BIT0;
    EUSCI_B3->TXBUF = adress;   //Send add
    while(!(EUSCI_B3->STATW & 1));


    while(!(EUSCI_B3->IFG & 2));
    EUSCI_B3->TXBUF = data;
    while(!(EUSCI_B3->STATW & 1));
    P10->OUT  &=~BIT0;
    SysTick_delay_us(1);
}

void SegInit(void){
    EUSCI_B3 -> CTLW0 |= 1;                  //Disable durring config
    EUSCI_B3 -> CTLW0 |= 0b1010100111000001;
                       //0b1010100111000001
    EUSCI_B3 -> BRW = 1000;
    EUSCI_B3 -> CTLW0 &=~0x01;                  //Enable EUSCI_B3

    //Configure Pins For SPI
    P10->SEL1 &=~(BIT1 | BIT2);
    P10->SEL0 |= (BIT1 | BIT2);

    //Configure Pins for GPIO (CS)
    P10->SEL1 &=~BIT0;
    P10->SEL0 &=~BIT0;
    P10->DIR  |= BIT0;
    P10->OUT  &=~BIT0;  //One mosfet, send inverted logic (start high)


    sendCommand(0x0A, 0x01);    //Intensity 2
    sendCommand(0x0B, 0x07);    //Set Scan limit
    sendCommand(0x0C, 0x01);    //Disable Shutdown
    sendCommand(0x09, 0x00);    //No Decode Mode
    int i,j;
    for(j = 0; j < 2; j++){
        for(i = 1; i < 7; i++){
            sendCommand(0x08, 1<<i);
            sendCommand(0x07, 1<<i);
            sendCommand(0x06, 1<<i);
            sendCommand(0x05, 1<<i);
            sendCommand(0x04, 1<<i);
            sendCommand(0x03, 1<<i);
            sendCommand(0x02, 1<<i);
            sendCommand(0x01, 1<<i);
            sendCommand(0x01, 1<<i);
            SysTick_delay_ms(50);
        }
    }

    sendCommand(0x09, 0xFF);    //Decode Mode
    sendCommand(0x08, 0x0f);    //
    sendCommand(0x07, 0x0f);    //
    sendCommand(0x06, 0x0C);    //H
    sendCommand(0x05, 0x0B);    //E
    sendCommand(0x04, 0x0D);    //L
    sendCommand(0x03, 0x0D);    //L
    sendCommand(0x02, 0x00);    //0
    sendCommand(0x01, 0x0f);    //
    sendCommand(0x01, 0x0f);    //

}


void displayDistance(double dist){
    int result;
    result = (int)dist;

    sendCommand(0x01, (int)((dist - result) * 10) % 10);


    if(dist > 1){
    result = ((int)dist % 10);
        sendCommand(0x02, result | BIT7);
    }
    else
        sendCommand(0x02, 0x0F);
    if(dist > 10){
        result = (((int)dist / 10) % 10);
        sendCommand(0x03, result);
    }
    else
        sendCommand(0x03, 0x0f);

    if(dist > 100){
        result = (((int)dist / 100) % 10);
        sendCommand(0x04, result);
    }
    else
        sendCommand(0x04, 0x0f);

}

void displayHz(int hz){

    if(hz > 1)
        sendCommand(0x05, (hz % 10));
    else
        sendCommand(0x05, 0x0F);
    if(hz > 10)
        sendCommand(0x06, (hz / 10) % 10);
    else
        sendCommand(0x06, 0x0f);

    if(hz > 100)
        sendCommand(0x07, (hz / 100) % 10);
    else
        sendCommand(0x07, 0x0f);
    if(hz >1000 )
        sendCommand(0x08, (hz / 1000) % 10);
    else
        sendCommand(0x08, 0x0f);
}


void animateSeg(){
    //8: F, E, D,
    //7: D,
    //7: D

    if(segAnim == 0)
        sendCommand(0x08, BIT1);
    else if(segAnim == 1)
        sendCommand(0x08, BIT2);
    else if(segAnim == 2)
        sendCommand(0x08, BIT3);
    else if(segAnim == 3){
        sendCommand(0x07, BIT3);
        sendCommand(0x08, 0x00);
    }
    else if(segAnim == 4){
        sendCommand(0x06, BIT3);
        sendCommand(0x07, 0x00);
    }
    else if(segAnim == 5){
        sendCommand(0x05, BIT3);
        sendCommand(0x06, 0x00);
    }
    else if(segAnim == 6){
        sendCommand(0x04, BIT3);
        sendCommand(0x05, 0x00);
    }
    else if(segAnim == 7){
        sendCommand(0x03, BIT3);
        sendCommand(0x04, 0x00);
    }
    else if(segAnim == 8){
        sendCommand(0x02, BIT3);
        sendCommand(0x03, 0x00);
    }
    else if(segAnim == 9){
        sendCommand(0x01, BIT3);
        sendCommand(0x02, 0x00);
    }
    else if(segAnim == 10){
        sendCommand(0x01, BIT4);
    }
    else if(segAnim == 11)
        sendCommand(0x01, BIT5);
    else if(segAnim == 12)
        sendCommand(0x01, BIT6);
    else if(segAnim == 13){
        sendCommand(0x02, BIT6);
        sendCommand(0x01, 0x00);
    }
    else if(segAnim == 14){
        sendCommand(0x02, 0x00);
        sendCommand(0x03, BIT6);
    }

    else if(segAnim == 15){
        sendCommand(0x03, 0x00);
        sendCommand(0x04, BIT6);
    }

    else if(segAnim == 16){
        sendCommand(0x04, 0x00);
        sendCommand(0x05, BIT6);
    }

    else if(segAnim == 17){
        sendCommand(0x05, 0x00);
        sendCommand(0x06, BIT6);
    }

    else if(segAnim == 18){
        sendCommand(0x06, 0x00);
        sendCommand(0x07, BIT6);
    }

    else if(segAnim == 19){
        sendCommand(0x07, 0x00);
        sendCommand(0x08, BIT6);
    }

}

void resetSeg(){
    sendCommand(0x08, 0x00);    //
    sendCommand(0x07, 0x00);    //
    sendCommand(0x06, 0x00);    //
    sendCommand(0x05, 0x00);    //
    sendCommand(0x04, 0x00);    //
    sendCommand(0x03, 0x00);    //
    sendCommand(0x02, 0x00);    //
    sendCommand(0x01, 0x00);    //
    sendCommand(0x01, 0x00);    //
}
