/**************************************************************************************
*                            ProximitySensor_Library.c
*                                Gabriel Gasbarre
*                                 October 20022
*
*     This is a library for control of a 5v, HC - SR04 Ultrasonic Ranging Module.
*
*
*
* **************************************************************************************/
#include "msp.h"
#include "ProximitySensor_Library.h"
#include "SysTick_Library12MHZ.h"
#include <math.h>
#include "Seg_Library.h"
#include <stdio.h>

int waiting = 0;
double distance = 0;
int play = 0;
/// ***************| getDistance |  *******************//*
/// * Brief: calculates the distance (inches) received by
/// *           the proximity sensor
/// * return:
/// *       double distance - inches received by prox.
/// ****************************************************/

//TODO OPTIMIZE THIS FUNCTION
void getDistance(void){
        if(!waiting){
            //Send Signal to Trig:
            P4->OUT  &= ~( BIT0 );  //Send low (inverse logic)
            SysTick_delay_us(10);   //Wait 10 micro seconds
            P4->OUT  |=  ( BIT0 );  //Send high
        }

        if(countReady){
            waiting = 0;
            countReady = 0;         //Reset flag

            //Once return signal is ready, run calculation
            double time = count * 0.33333;          //count to microseconds(.333 ms per cycle)
            time = time * pow(10, -6);              //microseconds to seconds
            distance = time * 13503.9;       //13503 inches per second
            distance /= 2;                          //Signal goes out and back, % by 2
            displayDistance(distance);
        }
}

/// ***************| getPercentDistance |***************//*
/// * Brief: calculates the percent of the maximum distance
/// *           the proximity sensor receives.
/// *           EX - max of 16 at 8 inches returns .5 for 50%
/// * param:
/// *       max - the maximum allowable distance which returns 1
/// * return:
/// *       double percentDist - percent of max distance
/// ****************************************************/
double getPercentDistance(int max){
    getDistance();
    double percentDist = distance / max; //Distance to %
    return percentDist;
}

/// ***************| prox_init |*8*********************//*
/// * Brief: initialize the proximity sensor on pins:
/// * 4.0 - TRIGGER PIN - GPIO OUTPUT, INVERSE LOGIC
/// * 6.6 - TA2.3 - INPUT CAPTURE PIN
/// ****************************************************/
void prox_init(void){
    toggle = 0;

    //Pin 6.7 Echo Pin
    P6->SEL0 |=  BIT6;    //TA0 Input Capture Pin
    P6->SEL1 &= ~BIT6;
    P6->DIR  &= ~BIT6;

    //Pin 4.0 Trigger Pin for Prox. Sensor:
    P4->SEL1 &=~( BIT0 );
    P4->SEL0 &=~( BIT0 );
    P4->DIR  |= ( BIT0 );
    P4->OUT  |= ( BIT0 );  //Start High because of inverse logic

    //Initialize TA2
    TIMER_A2->R = 0;
    //TIMER_A2->CTL |= TIMER_A_CTL_CLR;
    TIMER_A2->CTL      |= 0b001010100000;    //SMCLK, CONTINUOUS, CLEAR TA0R
    TIMER_A2->CCTL[3]  |= 0b1100100100010000; //CAPTURE RISING + FALLING EDGE,
    //USE CCI2A, ENABLE CAPTURE INTERRUPT, ENABLE CAPTURE MODE, SYNCHRONOUS CAPTURE
    //TIMER_A2->EX0     =   0b001;                  //Divide input clock to 3MHz


}


void TA2_N_IRQHandler(void)
{
    //Capture / Compare
    if(TIMER_A2->CCTL[3] & BIT0){
        if(!toggle){
            //Reset TA.0 on first call
            TIMER_A2->R = 0;
            waiting = 1;
            }

            else{
            //Save time
            count = TIMER_A2->CCR[3];
            countReady = 1;
            }

            toggle ^= 1;
            //Clear Flag
            TIMER_A2->CCTL[3] &= ~(TIMER_A_CCTLN_CCIFG);
    }
}
