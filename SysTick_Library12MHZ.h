/*
 * SysTick_Library.h
 *
 *  Created on: Sep 9, 2022
 *      Author: gasbarrg
 */
#include "msp.h"
#ifndef SYSTICK_LIBRARY12MHZ_H_
#define SYSTICK_LIBRARY12MHZ_H_

/********************* Macro Prototypes ********************************
 * SysTick Control and Status Register (STCSR) as discussed in lectures
***********************************************************************/
#define STCSR_CLKSRC    (0x0004)        // This is the CLKSOURSE bit, BIT2
#define STCSR_INT_EN    (0x0002)        // This is the TICKINT  bit, BIT1
#define STCSR_EN        (0x0001)        // This is the ENABLE bit, BIT0
/********************* Macro Prototypes ********************************
 *
 *
 *
***********************************************************************/


/********************* Global Flags *****************************
************************************************************************/
volatile uint8_t _flag;


/********************** Function Prototypes *****************************
************************************************************************/
void SysTickInit_WithInterrupts();
void SysTick_delay_us();
void SysTick_delay_ms();


#endif /* SYSTICK_LIBRARY12MHZ_H_ */
