
#include "msp.h"
#include "Motor_Library.h"
#include "Encoder_Library.h"
#include "SysTick_Library12MHZ.h"
#include <Seg_Library.h>
#include <stdio.h>

void motor_init(){

    P7->SEL0 &=~ (BIT4|BIT5|BIT6|BIT7);        //Sets GPIO
    P7->SEL1 &=~ (BIT4|BIT5|BIT6|BIT7);
    P7->DIR  |=  (BIT4|BIT5|BIT6|BIT7);       //Makes output
    P7->OUT  &=~ (BIT4|BIT5|BIT6|BIT7);        //Sets as low
}

void motor_zero_position(){

    int i;

    for(i = 0; i < 110; i++)
    {
                    P7->OUT |= (BIT4 | BIT7);
                    SysTick_delay_ms(2);
                    P7->OUT &=~ (BIT4);

                    P7->OUT |= (BIT5 | BIT7);
                    SysTick_delay_ms(2);
                    P7->OUT &=~ (BIT7);

                    P7->OUT |=  (BIT6 | BIT5);
                    SysTick_delay_ms(2);
                    P7->OUT &=~ (BIT5);

                    P7->OUT |= (BIT4 | BIT6);
                    SysTick_delay_ms(2);
                    P7->OUT &=~ (BIT6);
    }
}

void set_motor_position(int percent){

    int i;

    for(i = 0; i < percent*9; i++)
    {
        P7->OUT |= (BIT4 | BIT6);               //Setting BIT4 and BIT6 as high
        SysTick_delay_ms(2);                   //10 ms delay
        P7->OUT &=~ (BIT4);                     //Setting BIT4 as low

        P7->OUT |= (BIT6 | BIT5);
          SysTick_delay_ms(2);
          P7->OUT &=~ (BIT6);

        P7->OUT |= (BIT5 | BIT7);
          SysTick_delay_ms(2);
          P7->OUT &=~ (BIT5);

        P7->OUT |= (BIT4 | BIT7);
          SysTick_delay_ms(2);
          P7->OUT &=~ (BIT7);
    }
}


