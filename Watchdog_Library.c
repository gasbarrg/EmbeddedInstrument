/*
 * Watchdog_Library.c
 *
 *  Created on: Dec 8, 2022
 *      Author: Gabe
 */
#include "msp.h"
#include "Watchdog_Library.h"

void WDTinit(){
    //Initialize Pushbutton interrupt to ruin everything
    P1-> SEL0 &=~(BIT4);
    P1-> SEL1 &=~(BIT4);
    P1-> DIR  &=~(BIT4);           //make inputs
    P1-> REN  |= (BIT4);           //enable pull resistors
    P1-> OUT  |= (BIT4);          //pull down
    P1-> IES  |= BIT4;
    P1-> IE   |= BIT4;
    P1-> IFG = 0;

    //Initialize WDT
    CS->KEY = CS_KEY_VAL ;                          // Unlock CS module for register access
    CS->CTL1 |=  BIT9 | 0x2000000;                  //SELECT REFOCLK
    WDT_A->CTL = WDT_A_CTL_PW | 0b00101100;         //4 SEC @ 32KhZ

}

void kickWDT(){
    WDT_A->CTL = WDT_A_CTL_PW | 0b00101100;         //4 SEC @ 32KhZ
}

//Hold MSP in loop to trigger WDT
void PORT1_IRQHandler(){
    if(P1->IFG & BIT4)
        while(!(P1->IN & BIT4));
    P1->IFG = 0;
}




