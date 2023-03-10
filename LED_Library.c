/*
 * LED_Library.c
 *
 *  Created on: Sep 21, 2022
 *      Author: tfett
 */

#include "msp.h"
#include "LED_Library.h"


void Color_init()
{
    P2->SEL0 &=~ (BIT4|BIT5|BIT7);        //Sets GPIO
    P2->SEL1 &=~ (BIT4|BIT5|BIT7);
    P2->DIR  |=  (BIT4|BIT5|BIT7);        //Makes output
    P2->OUT  &=~ (BIT4|BIT5|BIT7);        //Sets as low

}
void RED(){

    P2->OUT  |=   BIT4;
    P2->OUT  &=~  BIT5;
    P2->OUT  &=~  BIT7;
}

void YELLOW(){

    P2->OUT  |=   BIT4;
    P2->OUT  |=   BIT5;
    P2->OUT  &=~  BIT7;

}

void GREEN(){

    P2->OUT  &=~  BIT4;
    P2->OUT  |=   BIT5;
    P2->OUT  &=~  BIT7;
}

void CYAN(){

    P2->OUT  &=~  BIT4;
    P2->OUT  |=   BIT5;
    P2->OUT  |=   BIT7;
}

void BLUE(){

    P2->OUT  &=~  BIT4;
    P2->OUT  &=~  BIT5;
    P2->OUT  |=   BIT7;
}

void PURPLE(){

    P2->OUT  |=   BIT4;
    P2->OUT  &=~  BIT5;
    P2->OUT  |=   BIT7;
}

void WHITE(){

    P2->OUT  |=   BIT4;
    P2->OUT  |=   BIT5;
    P2->OUT  |=   BIT7;
}




