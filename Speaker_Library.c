/**************************************************************************************
*                                Speaker_Libary.c
*                                Gabriel Gasbarre
*                                 October 20022
*
*     This is a library for control of an external speaker using Timer A on the MSP423
*
*
*
* **************************************************************************************/

#include "msp.h"
#include "Speaker_Library.h"
#include "ProximitySensor_Library.h"
#include <Seg_Library.h>
#include "LED_Library.h"
#include <SysTick_Library12MHZ.h>
#include "ST7735.h"
#include "Encoder_Library.h"
#include <string.h>
#include <stdio.h>


/// ***************| Speaker_init | *******************//*
/// * Brief: Initialize speaker using TA0 on pin 2.6
/// ****************************************************/
void Speaker_init(){
    //Initialize TA0
    TIMER_A0->CTL    |= 0b001001000100;         //SMCLK, UP, TIMER OFF, Enable interrupt
    TIMER_A0->EX0     = 0b011;                  //Divide input clock by 4 to 3MHz
    TIMER_A0->CCR[0]  = 65535;                  //assign period
    TIMER_A0->CCTL[3] = 0b11100000;             //Reset/Set
    TIMER_A0->CCR[3]  = 37500;                  //Assign duty cycle

    //Primary Output pin
    P2->SEL0 |=  BIT6;                          //Sets GPIO W/ TA
    P2->SEL1 &=~ BIT6;
    P2->DIR  |=  BIT6;                          //Makes output



    volatile int speakerToggle = 0;
}

//void TA0_N_IRQHandler(void)
//{
//    //Timer interrupt
//    if(TIMER_A0->CTL & BIT0){
//          //if(octave = 2)
//        TIMER_A0->CCR[3] = TIMER_A0->CCR[3] * 2;
//        TIMER_A0->CTL &= ~BIT0;    //Clear flag
//    }
//}


/// ***************| SpeakerOff | **********************//*
/// * Brief: Disable the speaker
/// ****************************************************/
void speakerOff(void){
    TIMER_A0->CTL   &=~ (BIT5 | BIT4);
}

/// ***************| SpeakerOn | ***********************//*
/// * Brief: Enable the speaker
/// ****************************************************/
void speakerOn(void){
    TIMER_A0->CTL   |= BIT4;
}

/// ***************| setHz | ***************************//*
/// * Brief: Set the frequency of the speaker
/// *
/// * param:
/// *       double Hz - frequency of note to play
/// ****************************************************/
void setHz(double Hz){
    //Assuming TA0 Running @ 1_500_000 cycles / sec
    TIMER_A0->CCR[0]  = (int)1500000/Hz;
}

/// ***************| playNote | ***********************//*
/// * Brief: Sets the frequency of the speaker and
/// *          automatically applies a duty cycle of
/// *          50%
/// *
/// * param:
/// *       double note - frequency of note to play
/// ****************************************************/
void playNote(double note){
    speakerOn();
    TIMER_A0->CCR[0]  = (int) 1500000/note;
    TIMER_A0->CCR[3] = (TIMER_A0->CCR[0] * dutyCycle) / 100;
}

/// ***************| playNote | ***********************//*
/// * Brief: Sets the duty cycle of the current note
/// *
/// * param:
/// *       int percent - duty cycle of note from 0-100
/// ****************************************************/
void setDutCycle(int percent){
    TIMER_A0->CCR[3] = (TIMER_A0->CCR[0] * percent) / 100;
}


/// ***************| playSclae | ***********************//*
/// * Brief: Takes the percent distance from the proximity
/// *           sensor and turns it into a note. Users may
/// *           select the lower and upper octave limits.
/// *
/// * param:
/// *       int Scale[] - desired scale of notes to be played
/// *       int max     - maximum octave
/// *       int min     - minimum octave
/// *       int size    - size (in notes) of the scale. usually
/// *                       defined as scale name in CAPS.
/// ****************************************************/
void playScale(const int Scale[], int max, int min, int size){
    double percDiff = getPercentDistance(maxDistance); //Get percent distance with max distance as x inches
    if(percDiff <= 1){
        int NPO = size / 8;                             //Notes Per Octave
        int totalNotes = (NPO * (max-min)) + 1;
        int noteLow    = (NPO * min);

        int note = (int)(totalNotes * percDiff);
        note += (int)noteLow;

        setHz(Scale[note]);
        LEDnotes(percDiff * 100);
        setDutCycle(dutyCycle);

        char lastNote[8];
        sprintf(lastNote, "%dHz     ", Scale[note]);
        ST7735_DrawString(11, 4, lastNote, TXTCOLOR);

        displayHz(Scale[note]);

        speakerOn();

    }

    //Turn off speaker if outside maximum range
    else
        speakerOff();
}

void LEDnotes(int percent)
{
    if(percent >= 0 && percent < 15)
            RED();
    if(percent >= 15 && percent < 30)
            YELLOW();
    if(percent >= 30 && percent < 45)
            GREEN();
    if(percent >= 45 && percent < 60)
            CYAN();
    if(percent >= 60 && percent < 75)
            BLUE();
    if(percent >= 75 && percent < 90)
            PURPLE();
    if(percent >= 90)
            WHITE();
}

void Clock_Init48MHz(void)
{
    // Configure Flash wait-state to 1 for both banks 0 & 1
       FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) |
       FLCTL_BANK0_RDCTL_WAIT_1;
       FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL & ~(FLCTL_BANK0_RDCTL_WAIT_MASK)) |
       FLCTL_BANK1_RDCTL_WAIT_1;

    //Configure HFXT to use 48MHz crystal, source to MCLK & HSMCLK*
       PJ->SEL0 |= BIT2 | BIT3;                     // Configure PJ.2/3 for HFXT function
       PJ->SEL1 &= ~(BIT2 | BIT3);
       CS->KEY = CS_KEY_VAL ;                       // Unlock CS module for register access
       CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;
          while(CS->IFG & CS_IFG_HFXTIFG)
                    CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;

    /* Select MCLK & HSMCLK = HFXT, no divider */
      CS->CTL1 = CS->CTL1 & ~(CS_CTL1_SELM_MASK     |
                              CS_CTL1_DIVM_MASK     |
                              CS_CTL1_SELS_MASK     |
                              CS_CTL1_DIVHS_MASK)   |
                              CS_CTL1_SELM__HFXTCLK |
                              CS_CTL1_SELS__HFXTCLK;

      CS->CTL1 = CS->CTL1 | CS_CTL1_DIVS_2;    // change the SMCLK clock speed to 12 MHz.

      CS->KEY = 0;                            // Lock CS module from unintended accesses
}





/// ***************| VARIABLES | ***********************//*
static const int
    C1 = 32.7,   CS1 = 34.6,   D1 = 36.7,   DS1 = 38.9,    E1 = 41.2,    F1 = 43.6,    FS1 = 46.3,    G1 = 49.0,    GS1 = 51.9,     A1 = 55.0,    AS1 = 58.3,    B1 = 61.7,
    C2 = 65.4,   CS2 = 69.3,   D2 = 73.4,   DS2 = 77.8,    E2 = 82.4,    F2 = 87.3,    FS2 = 92.5,    G2 = 98.0,    GS2 = 103.8,    A2 = 110.0,   AS2 = 116.5,   B2 = 123.5,
    C3 = 130.8,  CS3 = 138.59, D3 = 146.83, DS3 = 155.56,  E3 = 164.81,  F3 = 174.61,  FS3 = 185.0,   G3 = 196.0,   GS3 = 207.65,   A3 = 220.00,  AS3 = 233.08,  B3 = 246.94,
    C4 = 261.63, CS4 = 277.18, D4 = 293.66, DS4 = 311.13,  E4 = 329.63,  F4 = 349.2,   FS4 = 369.9,   G4 = 392.0,   GS4 = 415.3,    A4 = 440.0,   AS4 = 466.1,   B4 = 493.8,
    C5 = 523.2,  CS5 = 554.3,  D5 = 587.3,  DS5 = 622.2,   E5 = 659.2,   F5 = 698.4,   FS5 = 739.9,   G5 = 783.9,   GS5 = 830.6,    A5 = 880.0,   AS5 = 932.3,   B5 = 987.7,
    C6 = 1046.5, CS6 = 1108.7, D6 = 1174.6, DS6 = 1244.5,  E6 = 1318.5,  F6 = 1396.9,  FS6 = 1479.9,  G6 = 1567.9,  GS6 = 1661.2,   A6 = 1760.0,  AS6 = 1864.6,  B6 = 1975.5,
    C7 = 2093.0, CS7 = 2217.4, D7 = 2349.3, DS7 = 2489.0,  E7 = 2637.0,  F7 = 2793.8,  FS7 = 2959.9,  G7 = 3135.9,  GS7 = 3135.9,   A7 = 3520.00, AS7 = 3729.31, B7 = 3951.07,
    C8 = 4186.0, CS8 = 4434.9, D8 = 4698.6, DS8 = 4978.03, E8 = 5274.04, F8 = 5587.65, FS8 = 5919.91, G8 = 6271.93, GS8 = 6644.88,  A8 = 7040.00, AS8 = 7458.62, B8 = 7902.13;


int notes[NOTES] = {            //96 Notes
    C1, CS1, D1, DS1, E1, F1, FS1, G1, GS1, A1, AS1, B1,
    C2, CS2, D2, DS2, E2, F2, FS2, G2, GS2, A2, AS2, B2,
    C3, CS3, D3, DS3, E3, F3, FS3, G3, GS3, A3, AS3, B3,
    C4, CS4, D4, DS4, E4, F4, FS4, G4, GS4, A4, AS4, B4,
    C5, CS5, D5, DS5, E5, F5, FS5, G5, GS5, A5, AS5, B5,
    C6, CS6, D6, DS6, E6, F6, FS6, G6, GS6, A6, AS6, B6,
    C7, CS7, D7, DS7, E7, F7, FS7, G7, GS7, A7, AS7, B7,
    C8, CS8, D8, DS8, E8, F8, FS8, G8, GS8, A8, AS8, B8,
};
int C_Major[C_MAJOR] = {        //56 Notes
    C1, D1, E1, F1, G1, A1, B1,
    C2, D2, E2, F2, G2, A2, B2,
    C3, D3, E3, F3, G3, A3, B3,
    C4, D4, E4, F4, G4, A4, B4,
    C5, D5, E5, F5, G5, A5, B5,
    C6, D6, E6, F6, G6, A6, B6,
    C7, D7, E7, F7, G7, A7, B7,
    C8, D8, E8, F8, G8, A8, B8,
};
int C_Minor[C_MINOR] = {        //56 Notes
    C1, D1, DS1, F1, G1, GS1, AS1,
    C2, D2, DS2, F2, G2, GS2, AS2,
    C3, D3, DS3, F3, G3, GS3, AS3,
    C4, D4, DS4, F4, G4, GS4, AS4,
    C5, D5, DS5, F5, G5, GS5, AS5,
    C6, D6, DS6, F6, G6, GS6, AS6,
    C7, D7, DS7, F7, G7, GS7, AS7,
    C8, D8, DS8, F8, G8, GS8, AS8,
};
int Blues[BLUES] = {            //48 Notes
    C1, DS1, F1, FS1, G1, AS1,
    C2, DS2, F2, FS2, G2, AS2,
    C3, DS3, F3, FS3, G3, AS3,
    C4, DS4, F4, FS4, G4, AS4,
    C5, DS5, F5, FS5, G5, AS5,
    C6, DS6, F6, FS6, G6, AS6,
    C7, DS7, F7, FS7, G7, AS7,
    C8, DS8, F8, FS8, G8, AS8,
};
int Major_Pentatonic[MAJOR_PENTATONIC] = {
    C1, D1, E1, G1, A1,
    C2, D2, E2, G2, A2,
    C3, D3, E3, G3, A3,
    C4, D4, E4, G4, A4,
    C5, D5, E5, G5, A5,
    C6, D6, E6, G6, A6,
    C7, D7, E7, G7, A7,
    C8, D8, E8, G8, A8,
};

void funkyTown(void){
    int i;

    for(i = 0; i < 8; i++){
    playNote(C2);
    SysTick_delay_ms(250);
    playNote(C3);
    SysTick_delay_ms(250);
    }

    for(i = 0; i < 2; i++){
    playNote(C6);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    playNote(C6);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    playNote(AS5);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    playNote(C6);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    SysTick_delay_ms(125);
    SysTick_delay_ms(125);
    playNote(G5);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    SysTick_delay_ms(125);
    SysTick_delay_ms(125);
    playNote(G5);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    playNote(C6);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    playNote(F6);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    playNote(E6);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    playNote(C6);
    SysTick_delay_ms(125);
    speakerOff();
    SysTick_delay_ms(125);
    SysTick_delay_ms(250);
    SysTick_delay_ms(250);
    SysTick_delay_ms(250);
    SysTick_delay_ms(250);
    }
}

void funkyTown2(void){
    playNote(C6);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    playNote(C6);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    playNote(AS5);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    playNote(C6);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    SysTick_delay_ms(100);
    SysTick_delay_ms(100);
    playNote(G5);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    SysTick_delay_ms(100);
    SysTick_delay_ms(100);
    playNote(G5);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    playNote(C6);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    playNote(F6);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    playNote(E6);
    SysTick_delay_ms(100);
    speakerOff();
    SysTick_delay_ms(100);
    playNote(C6);
    SysTick_delay_ms(100);
    speakerOff();
}
