/*
 * SysTick_Library.c
 *
 *  Created on: Sep 9, 2022
 *      Author: gasbarrg
 */



#include <SysTick_Library12MHZ.h>
#include "msp.h"


void SysTickInit_WithInterrupts(uint32_t delay_ms){
    SysTick->CTRL  &= ~BIT0;                                             //clears enable to stop the counter
    SysTick->LOAD   = delay_ms * 48000;                                  //sets the period
    SysTick->VAL    = 0;                                                 //clears the value
    SysTick->CTRL   = (STCSR_CLKSRC | STCSR_INT_EN | STCSR_EN);          //enable SysTick with core clock, interrupts on -> this is the ENABLE, TICKINT, and CLKSOURSE bits: Systic->CTRL |= 0x07;
}

void SysTick_delay_ms(uint32_t ms_delay){
    //Delays time_ms number of milliseconds
    //Assume 3MHz clock -> 3000 cycles per millisecond
    SysTick->LOAD  = 48000 * (uint32_t)ms_delay;
    SysTick->VAL   = 0;                               // starts counting from 0
    SysTick->CTRL |= (STCSR_CLKSRC | STCSR_EN);       // ENABLE, CLKSOURSE bits  .... Systic->CTRL |= 0x05;
    while(!(SysTick->CTRL & ((uint32_t)1)<<16));      // Continue while bit 16 is high or use   ....while( (SysTick->CTRL & BIT16) == 0);
    SysTick->CTRL &= ~(STCSR_CLKSRC | STCSR_EN);      // Disable the Systic timer               .... Systic->CTRL =0 ;
}

void SysTick_delay_us(uint32_t us_delay){
    //Delays time_ms number of milliseconds
    //Assume 3MHz clock -> 3 cycles per microsecond
    SysTick->LOAD  = us_delay*48 - 1;                //counts up to delay
    SysTick->VAL   = 0;                              //starts counting from 0
    SysTick->CTRL |= (STCSR_CLKSRC | STCSR_EN);      // ENABLE, CLKSOURSE bits  .... Systic->CTRL |= 0x05;
    while(!(SysTick->CTRL & ((uint32_t)1)<<16));     // Continue while bit 16 is high   .... while( (SysTick->CTRL & BIT16) == 0);
    SysTick->CTRL &= ~(STCSR_CLKSRC | STCSR_EN);     // Disable the Systic timer        .... Systic->CTRL =0 ;
}

/// ****| SysTick_Handler | **************************//*
/// * Brief: SysTick Handler (rewrite for desired use)
/// *
/// * param:
/// * N/A
/// * return:
/// * N/A
/// ****************************************************/
void SysTick_Handler(void) {

    _flag = 1;

    //turn off

    SysTick->CTRL &= ~BIT0;
}

