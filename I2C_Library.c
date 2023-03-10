/*
 * I2C_Library.c
 *
 *  Created on: Sep 30, 2022
 *      Author: gasbarrg
 */
#include "msp.h"
#include "I2C_Library.h"
#include <stdio.h>

extern int slaveAddr = 0b1101000;              //Slave Addr = 0x68
extern unsigned char dateTimeReturn[7] = "";
                             //0-60  0-60  1-12   1-7  1-31  1-12  0-99
                             //Sec    Min   Hrs   Day  Date  Month Year
extern unsigned char dateTime[7]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                             //   0     1     2     3     4     5     6
//Note, bit 6 of Hrs needs to be 1 for 12 hours, and bit 5 needs to be 1 for PM, 0 for AM

//Convert Read value to a Day:
const extern char *days[7]    = {"Sunday", "Monday", "Tuesday", "Weds.", "Thursday", "Friday", "Saturday"};
//Convert Read value to Months
const extern char *months[19] = {"January", "February", "March", "April", "May", "June", "July", "August",
                           "Sept.",
                           "","","","","","",    //Empty BC 10 hex = 16
                           "October", "November", "December" };

void RTC_init(void){

    EUSCI_B1->CTLW0 |= 1;               //Disable during configuration
    EUSCI_B1->CTLW0 = 0b111110000001;   //7 bit slave addr, master, i2c, synch mode, use smclk
    EUSCI_B1->BRW   = 5000;              // UCBR = baud rate set clk prescaler 48MHz / 480 = 100KHz
    P6->SEL0 |=  (BIT4 | BIT5);
    P6->SEL1 &= ~(BIT4 | BIT5);
    EUSCI_B1->CTLW0 &= ~0x0001;         // enable the USCI module

}


//EUSCI_B1->CTLW0 |= EUSCI_B_CTLW0_SWRST;                 // Hold EUSCI_B1 module in reset state
//EUSCI_B1->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;                // Clear SWRST to resume operation

int RTC_Write(int slaveAddr, unsigned char memAddr, unsigned char data){

    EUSCI_B1->I2CSA  = slaveAddr;      //Assign slave address
    EUSCI_B1->CTLW0 |= 0x0010;         //Enable transmitter
    EUSCI_B1->CTLW0 |= 0x0002;         //START, send slave addr
    while(EUSCI_B1->CTLW0 & 2);        //Wait for ACK
    EUSCI_B1->TXBUF = memAddr;         //Send memAddr
    while(!(EUSCI_B1->IFG & 2));       //while(EUSCI_B1->CTLW0 & 2);          //Wait for ACK
    EUSCI_B1->TXBUF = data;            //Send data
    while(!(EUSCI_B1->IFG & 2));       //while(EUSCI_B1->CTLW0 & 2);          //Wait for ACK
    EUSCI_B1->CTLW0 |= 0x0004;         //Send Stop
    while(EUSCI_B1->CTLW0 & 4);        //Wait till stop sent

    return 0;                          //No errors
}

int RTC_Read(int slaveAddr, unsigned char memAddr, unsigned char* data){
    EUSCI_B1->I2CSA  = slaveAddr;      //Assign slave address
    EUSCI_B1->CTLW0 |= 0x0010;         //Enable transmitter
    EUSCI_B1->CTLW0 |= 0x0002;         //START, send slave addr

    while(EUSCI_B1->CTLW0 & 2);        //Wait for ACK
    EUSCI_B1->TXBUF = memAddr;         //Send memAddr
    while(!(EUSCI_B1->IFG & 2));       //while(EUSCI_B1->CTLW0 & 2);          //Wait for ACK
    EUSCI_B1->CTLW0 &= ~0x0010;        //Enable Receiver
    EUSCI_B1->CTLW0 |= 0x0002;         //RE-START,
    while(EUSCI_B1->CTLW0 & 2);        //Wait for restart
    EUSCI_B1->CTLW0 |= 0x0004;         //Send Stop after data received

    while(!(EUSCI_B1->IFG & 1));
    //while((EUSCI_B1->CTLW0 & 1));    //Wait for data received
    *data = EUSCI_B1->RXBUF;           //Read Recieved data, send to pointer
    //printf("Read Data: 0x%x\n", EUSCI_B1->RXBUF);
    //printf("Saved Data: 0x%x\n", *data);
    while(EUSCI_B1->CTLW0 & 4);        //Wait till stop sent


    return 0;                          //No errors
}


void RTC_BurstWrite(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data){
    if (byteCount <= 0)
        return -1;

      EUSCI_B1->I2CSA = slaveAddr;    //Setup Slave Addr
      EUSCI_B1->CTLW0 |= 0x0010;      //enable transmitter
      EUSCI_B1->CTLW0 |= 0x0002;      //generate START and send slave addr
      while (EUSCI_B1->CTLW0 & 2);    //wait until ready to transmit
      EUSCI_B1->TXBUF = memAddr;      //send memory address to slave


      do {                                //send data one bit at a time
          while(!(EUSCI_B1->IFG & 2));    //wait until ready to transmit
          EUSCI_B1->TXBUF = *data++;      //send data to slave
          byteCount--;
      } while(byteCount > 0);

      while(!(EUSCI_B1->IFG & 2));    //wait until last transmit is done
      EUSCI_B1->CTLW0 |= 0x0004;      //send STOP
      while(EUSCI_B1->CTLW0 & 4);     //wait until stop is sent

      return 0;                       //no error
}


int RTC_BurstRead(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data){
    if (byteCount <= 0) {return -1;}    //no read was performed

    EUSCI_B1->I2CSA = slaveAddr;        //setup slave addr
    EUSCI_B1->CTLW0 |= 0x0010;          //enable transmitter
    EUSCI_B1->CTLW0 |= 0x0002;          //generate START and send slave addr
    while (EUSCI_B1->CTLW0 &2);         //wait until RESTART is finished
    EUSCI_B1->TXBUF = memAddr;          //send memory to slave
    while (!(EUSCI_B1->IFG & 2));       //wait until last transmit is complete
    EUSCI_B1->CTLW0 &= ~0x0010;         //enable receiver
    EUSCI_B1->CTLW0 |= 0x0002;          //generate RESTART and send slave address
    while (EUSCI_B1->CTLW0 &2);         //wait until RESTART is finished

    do {                                //receive data one byte at a time
       if(byteCount == 1)              //only when 1 byte data is left
            EUSCI_B1->CTLW0 |= 0x0004;  //setup to send stop after last byte is recieved

        while(!(EUSCI_B1->IFG & 1));    //wait until data is recieved
        *data++ = EUSCI_B1->RXBUF;      //read recieved data
        byteCount--;
    } while(byteCount);

    while(EUSCI_B1->CTLW0 & 4);         //wait until stop bit is sent

    return 0;                           //no error

}


float RTC_TempRead(){
    unsigned char tempRead;
    double tempReal;
    //Get integer portion of temperature:
    RTC_Read(0x68, 0x11, &tempRead);
    //Save it
    tempReal = tempRead;
    //Get fractional portion:
    RTC_Read(0x68, 0x12, &tempRead);
    //convert
    tempRead = tempRead >> 6;
    //save it
    tempReal += tempRead * 0.25;

    return tempReal;
}
