/**************************************************************************************
*                           EGR 327-901 Final Project
*                                    main.c
*
*                     Gabriel Gasbarre and Taylor Fettig
*                                 October 2022
*                                   Dr. Krug
*
* **************************************************************************************/

//Library initializations
#include "BMPs.h"
#include "msp.h"
#include <math.h>
#include <stdio.h>
#include "ST7735.h"
#include "I2C_Library.h"
#include "LCD_Library.h"
#include "Seg_Library.h"
#include "LED_Library.h"
#include "Motor_Library.h"
#include "Encoder_Library.h"
#include "Speaker_Library.h"
#include "Watchdog_Library.h"
#include "SysTick_Library12MHZ.h"
#include "ProximitySensor_Library.h"


/// ***********| USER-DEFINED VARIABLES | ***************//*

int maxDistance =  20;           //Maximum distance from proximity sensor;
int dutyCycle   =  50;           //desired Duty Cycle percent
int volume;
int scale = 0;
int lowOct = 2;
int highOct = 4;


enum finalproject {
    main_menu,
    date_time_setup,
    instrument_config,
    play_instrument };

//Starts state machine at the main menu
enum finalproject state = main_menu;

void main(void)



{
    //STOP WDT BEFORE INITIALIZATIONS:
    WDT_A->CTL = WDT_A_CTL_PW |   WDT_A_CTL_HOLD;

    //Initializations;
    Clock_Init48MHz();                                               //Clock set to high frequency to run LCD
    ST7735_InitR(INITR_BLACKTAB);                                    //Initialization for ST7735R screens (green or red tabs).
    prox_init();
    Speaker_init();
    speakerOff();
    Encoder_Init();
    RTC_init();
    Color_init();
    motor_init();


    //Interrupt Vectors:
    NVIC->ISER[0] = 1 << ((TA2_N_IRQn) & 31);       //Enable TA 2 Interrupt vector
    NVIC->ISER[0] = 1 << ((TA0_N_IRQn) & 31);       //Enable TA 0 Interrupt vector
    NVIC->ISER[0] |= 1 <<((T32_INT1_IRQn) & 31 );
    NVIC->ISER[0] |= 1 <<((T32_INT2_IRQn) & 31 );
    NVIC_EnableIRQ(PORT1_IRQn);
    __enable_irq();


    //PLAY BOOT UP ANIMATIONS:
    ST7735_FillScreen(0x39E6);
    ST7735_DrawBitmap(45, 85, VolumeOn, 30, 21);                  //PRINT MAIN MENU PICTURE
    SegInit();
    funkyTown2();
    motor_zero_position();
    set_motor_position(10);

    //Finally, enable WDT
    WDTinit();


    while (1)
    {
        switch(state)
        {

        case main_menu:                                              //Program starts at main menu

            ST7735_DrawBitmap(0, 160, MainMenu, 128, 160);                  //PRINT MAIN MENU PICTURE
            ST7735_DrawString(0, 10, "  Date & Time Setup", TXTCOLOR);        //String to select time setup
            ST7735_DrawString(0, 12, " Instrument Settings", TXTCOLOR);      //String to select instrument configuration
            ST7735_DrawString(0, 14,"  Toggle Instrument", TXTCOLOR);

            //PRINT TEMP AND LAST NOTE
            ST7735_DrawString(1, 4, "LAST NOTE:", TXTCOLOR);
            ST7735_DrawString(1, 7, "TEMPERATURE:", TXTCOLOR);

            speakerOff();
            //PRINT VOLUME ON
            ST7735_DrawBitmap(93, 85, VolumeOff, 30, 21);                  //PRINT MAIN MENU PICTURE

            //Get RTC date time
            getDate();
            getTemp();

            //prepare 7-seg
            segAnim = 0;
            resetSeg();
            sendCommand(0x09, 0x00);    //No Decode Mode

            int next = 0;                   //Next Menu item
            while(!SW_FLAG){
                kickWDT();                  //RESET WATCHDOG
                animateSeg();
                scrollDate();
                next = encoderSelect();
            }
            waitSW(); //wait for encoder pushbutton

            if(!next)
                state = date_time_setup;
            else if(next == 1)
                state = instrument_config;
            else{
                state = play_instrument;
            }

            break;

        case date_time_setup:               //Date and time menu
            /**
             *  prints the menu overlay and then goes through each year, month,
             *  day ect. Continuously polling encoder until selection is made.
             */

            inMenu = 1; //Notify timeout counter to begin

            YEARMenu();
            while(!SW_FLAG && !rtcTimeout){
                kickWDT();                  //RESET WATCHDOG
                scrollYear();
            }
            waitSW();                       //wait for encoder pushbutton

            MONTHMenu();
            while(!SW_FLAG  && !rtcTimeout){
                kickWDT();                  //RESET WATCHDOG
                scrollMonth();
            }
            waitSW();                       //wait for encoder pushbutton

            DATEMenu();
            while(!SW_FLAG && !rtcTimeout){
                kickWDT();                  //RESET WATCHDOG
                scrollDate2();
            }
            waitSW();                       //wait for encoder pushbutton

            DAYMenu();
            while(!SW_FLAG && !rtcTimeout){
                kickWDT();                  //RESET WATCHDOG
                scrollDay();
            }
            waitSW();                       //wait for encoder pushbutton

            HOURMenu();
            while(!SW_FLAG && !rtcTimeout){
                kickWDT();                  //RESET WATCHDOG
                scrollHour();
            }
            waitSW();                       //wait for encoder pushbutton

            MINUTEMenu();
            while(!SW_FLAG && !rtcTimeout){
                kickWDT();                  //RESET WATCHDOG
                scrollMinute();
            }
            waitSW();                       //wait for encoder pushbutton

            SECMenu();
            while(!SW_FLAG && !rtcTimeout){
                kickWDT();                  //RESET WATCHDOG
                scrollSec();
            }
            waitSW();                       //wait for encoder pushbutton

            //send the data received to the rotary encoder
            if(!rtcTimeout)
                sendTime();

            //Reset menu timeout counts
            timeoutCount = 0;
            inMenu = 0;
            rtcTimeout = 0;

            //Back to main menu
            state = main_menu;
            break;

        case  instrument_config:                                    //Instrument configuration menu

            VOLMenu();
            while(!SW_FLAG){
                kickWDT();                  //RESET WATCHDOG
                volume = scrollVol();
            }
            waitSW(); //wait for encoder pushbutton


            //reset motor then move to new volume            motor_zero_position();
            set_motor_position(volume);

            SCALEMenu();
            while(!SW_FLAG){
                animateSeg();
                kickWDT();                  //RESET WATCHDOG
                scale = scrollScales();
            }
            waitSW(); //wait for encoder pushbutton

            LowOctMenu();
            while(!SW_FLAG){
                animateSeg();
                kickWDT();                  //RESET WATCHDOG
                lowOct = scrollLowOct();
            }
            waitSW(); //wait for encoder pushbutton

            HighOctMenu();
            while(!SW_FLAG){
                animateSeg();
                kickWDT();                  //RESET WATCHDOG
                highOct = scrollHighOct();
            }
            waitSW(); //wait for encoder pushbutton

            //Return to main menu
            state = main_menu;
            break;

        case  play_instrument:                                      //Instrument playing home screen
            //PRINT VOLUME ON
            ST7735_DrawBitmap(93, 85, VolumeOn, 30, 21);

            sendCommand(0x09, 0xFF);    //Decode Mode
            sendCommand(0x06, 0x0F);    //H
            sendCommand(0x05, 0x0F);    //E
            sendCommand(0x04, 0x0F);    //L


            while(!SW_FLAG){
                kickWDT();                  //RESET WATCHDOG
                scrollDate();
                if(scale == 0 )
                    playScale(Blues, lowOct, highOct, BLUES);
                else if(scale == 1)
                    playScale(C_Major, lowOct, highOct, C_MAJOR);
                else if(scale == 2)
                    playScale(C_Minor, lowOct, highOct, C_MINOR);
                else
                    playScale(Major_Pentatonic, lowOct, highOct, MAJOR_PENTATONIC);
            }
            waitSW(); //wait for encoder pushbutton

            //Return to main menu
            state = main_menu;
            break;


        }
    }
}

