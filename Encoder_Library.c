/*
 * Encoder_Library.c
 *
 *  Created on: Sep 23, 2022
 *      Author: gasbarrg
 *
 *
 *
 *
 *
 *
 */
#include "msp.h"
#include "Encoder_Library.h"
#include "Speaker_Library.h"
#include "SysTick_Library12MHz.h"
#include "ST7735.h"
#include "BMPs.h"
#include "I2C_Library.h"
#include "LCD_Library.h"
#include "Motor_Library.h"
#include "math.h"
#include <stdio.h>

//DATE TIME VARIABLES TO SEND TO RTC:
unsigned char RTCyear, RTCmonth, RTCdate, RTCday, RTChour, RTCmin, RTCsec;
extern int rtcTimeout = 0;
extern int timeoutCount = 0;
extern int inMenu = 0;
extern int segAnim = 0;

const extern char *scales[7] = {"Blues", "C Major", "C Minor", "M. Pent."};

//used to ensure ample time for encoder gates to close
void T32_Oneshot(void){
    TIMER32_1->CONTROL &= ~BIT7;                //Disable
    TIMER32_1->LOAD     = 8 * 3000;             //RELOAD VALUE x * 3000
    TIMER32_1->CONTROL |= 0b01100111;           //DISABLE, PERIOIC, INTERRUPT, 0 , /16, 32BIT, WRAPPING MODE
    TIMER32_1->INTCLR   = 0x0;                  //Clear flag
    TIMER32_1->CONTROL |= BIT7;                 //Enable
}
void T32_Periodic(void){
    TIMER32_2->CONTROL &= ~BIT7;                //Disable
    TIMER32_2->LOAD     = 200 * 3000;            //RELOAD VALUE x * 3000
    TIMER32_2->CONTROL |= 0b01100110;           //DISABLE, PERIOIC, INTERRUPT, 0 , /16, 32BIT, WRAPPING MODE
    TIMER32_2->INTCLR   = 0x0;                  //Clear flag
    TIMER32_2->CONTROL |= BIT7;                 //Enable


}

void Encoder_Init(void){

    //Initialize CLK, DT, and SW as pull-up inputs:
    P4-> SEL0 &=~(BIT4 | BIT5 | BIT7);
    P4-> SEL1 &=~(BIT4 | BIT5 | BIT7);          //configure as GPIO
    P4-> DIR  &=~(BIT4 | BIT5 | BIT7);          //make inputs
    P4-> REN  |= (BIT4 | BIT5 | BIT7);          //enable pull resistors
    P4-> OUT  |= (BIT4 | BIT5 | BIT7);          //pull up

    //Enable Interrupts
    P4->IES &= ~(BIT4);                  //Interrupt on Low-High
    P4->IES |= (BIT7);                   //Interrupt on High-Low
    P4->IE  |=  (BIT4 | BIT7);           //Interrupt Enable

    //Clear Interrupt Flag
    P4->IFG = 0;

    _encoderReady = 0;
    year = 2022;
    count = -1;
    NVIC_EnableIRQ(PORT4_IRQn);                 //Enable Port 4 Interrupts

    //Enable T32
    T32_Oneshot();
    T32_Periodic();
}



void T32_INT1_IRQHandler(void){
    TIMER32_1->INTCLR = 0x0; // clear flag
    _encoderReady = 1;
}

void T32_INT2_IRQHandler(void){
    TIMER32_2->INTCLR = 0x0; // clear flag
    _scrollReady = 1;


    //RTC menu timeout variable, needs to increment to
    //.2(s) x count = 60 --> 300 counts = 1 minute to timeout
    if(inMenu)
        timeoutCount++;
    if(timeoutCount == 300)
        rtcTimeout = 1;
    segAnim++;
    if(segAnim == 20)
        segAnim = 0;
}

int compareClocks(void){
    //Get Output from CLK and DT, return 1 = CW; 0 = CCW
    int CLK = ((P4->IN & BIT4) == BIT4);
    int DT = ((P4->IN & BIT5) == BIT5);
    if(CLK == DT)return(0);
    else return(1);
}

//Encoder Interrupt Routine
void PORT4_IRQHandler(void){

    //MUST HAVE AT LEAST 8 MS IN BETWEEN CALLS
    //SysTick initialized with a 5ms Interrupt. If the program tries to set the CLK flag
    //within 5ms of the last call to the interrupt handler, the encoder is moving too fast.
    //Thus skip any assignments and re-initalize SysTick to force program to wait at least
    //another 5ms.

    //Check T32 Flag is set
    if(_encoderReady){
        //Clear T32 flag
        _encoderReady =0;
        //CLK Flag:
        if(P4->IFG & BIT4){
            CLK_FLAG = 1;
            clkOutput = compareClocks();
            //Toggle interrupt edge select
            P4->IES ^= BIT4;
        }
    }

    if(P4->IFG & BIT7)
        SW_FLAG = 1;

    //Reset T32
    T32_Oneshot();

    //Clear Interrupt Flag
    P4->IFG = 0;
}


void waitSW(void){
    SysTick_delay_ms(10);
    while(!(P4->IN & BIT7)){};
    SysTick_delay_ms(10);
    SW_FLAG = 0;
}


void resetSquares(void){
    if(lstCount == 0){
        ST7735_DrawString(0, 10, "  Date & Time Setup", TXTCOLOR);        //String to select time setup

    }
    else if(lstCount == 1){
        ST7735_DrawString(0, 12, " Instrument Settings", TXTCOLOR);      //String to select instrument configuration
    }
    else{
        ST7735_DrawString(0, 14,"  Toggle Instrument", TXTCOLOR);     //Start instrument
    }
}


uint8_t encoderSelect(void){

    //Changes the main menu selection if the rotary encoder has been moved
    if(CLK_FLAG) {
        CLK_FLAG = 0;

        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        //clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        //Force count to loop 0-2
        if(count < 0)
            count = 2;
        if(count > 2)
            count = 0;

        if(count == 0)
        {
            resetSquares();
            ST7735_DrawString(0, 10, "  Date & Time Setup", 0x0236);        //String to select time setup
        }
        else if(count == 1)
        {
            resetSquares();
            ST7735_DrawString(0, 12, " Instrument Settings", 0x0236);      //String to select instrument configuration
        }
        else if(count == 2)
        {
            resetSquares();
            ST7735_DrawString(0, 14,"  Toggle Instrument", 0x0236);     //Start instrument
        }

        //P4->IE  |=  (BIT4 | BIT7);
        lstCount = count;
    }
    return count; //Return next menu selection
}

void scrollYear(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        year = 2022 + count;

        //CONVERT NUMBER TO STRING
        char yr1[5], yr2[5], yr3[5];
        sprintf(yr1, "%d", year - 1);
        sprintf(yr2, "%d", year);
        sprintf(yr3, "%d", year + 1);
        //SMALL TOP TEXT
        ST7735_DrawString(8, 8, yr1, TXTCOLOR);
        //lARGE MIDDLE TEXT
        ST7735_DrawString2(3, 9, yr2, TXTCOLOR);
        //SMALL BOTTOM TEXT
        ST7735_DrawString(8, 11, yr3, TXTCOLOR);


    }
    RTCyear = count + 22;
}

void YEARMenu(void){
    count = 0;
    ST7735_DrawBitmap(0, 160 , datetime, 128, 160);

    //              X   Y   W    H
    ST7735_DrawString(1, 2, "Date and Time Setup", TXTCOLOR);

    ST7735_DrawString(0, 5, "     SELECT YEAR    ", TXTCOLOR);


    //SMALL TOP TEXT
    ST7735_DrawString(8, 8, "2021", TXTCOLOR);
    //lARGE MIDDLE TEXT
    ST7735_DrawString2(3, 9, "2022", TXTCOLOR);
    //SMALL BOTTOM TEXT
    ST7735_DrawString(8, 11, "2023", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(1, 149, 16, 10, TXTCOLOR); //1 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}


void scrollMonth(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;


        //shift count if month > 8
        if (count == 9)
            count += 6;
        if (count == 14)
            count -= 6;
        if (count > 17)
            count = 0;
        if (count < 0)
            count = 17;
        //SMALL TOP TEXT

        //lARGE MIDDLE TEXT
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(1, 9, months[count], TXTCOLOR);
        //SMALL BOTTOM TEXT

    }
    if(count > 9)
        RTCmonth = count - 5;
    else
        RTCmonth = count + 1;
}
void MONTHMenu(void){
    count = 0;
    ST7735_FillRect(1, 44, 126, 18, ACC2COLOR);
    ST7735_DrawString(0, 5, "    SELECT MONTH    ", TXTCOLOR);

    //SMALL TOP TEXT
    ST7735_DrawString(8, 8, "        ", TXTCOLOR);
    //lARGE MIDDLE TEXT
    ST7735_DrawString2(2, 9, "January", TXTCOLOR);
    //SMALL BOTTOM TEXT
    ST7735_DrawString(8, 11,"        ", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(18, 149, 17, 10, TXTCOLOR); //2 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

void scrollDate2(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 31){
            count = 1;
        }
        if(count < 1){
            count = 31;
        }

        char date1[5];
        sprintf(date1, "%d", count);
        //lARGE MIDDLE TEXT
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(4, 9, date1, TXTCOLOR);

    }
     RTCdate = count;
}
void DATEMenu(void){
    count = 1;
    ST7735_FillRect(1, 44, 126, 18, ACC2COLOR);
    ST7735_DrawString(0, 5, "    SELECT DATE    ", TXTCOLOR);

    //lARGE MIDDLE TEXT
    ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
    ST7735_DrawString2(4, 9, "1", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(37, 149, 17, 10, TXTCOLOR); //2 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);

}

void scrollDay(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if (count > 6)
            count = 0;
        if (count < 0)
            count = 6;

        //lARGE MIDDLE TEXT
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(1, 9, days[count], TXTCOLOR);

    }
    RTCday = count + 1;
}

void DAYMenu(void){
    count = 0;
    ST7735_FillRect(1, 44, 126, 18, ACC2COLOR);
    ST7735_DrawString(0, 5, "     SELECT DAY    ", TXTCOLOR);

    //lARGE MIDDLE TEXT
    ST7735_DrawString2(1, 9, "Sunday", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(56, 149, 17, 10, TXTCOLOR); //2 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

void scrollHour(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 12){
            count = 1;
        }
        if(count < 1){
            count = 12;
        }

        char date1[5];
        sprintf(date1, "%d", count);
        //lARGE MIDDLE TEXT
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(4, 9, date1, TXTCOLOR);


    }
    RTChour = count;
}

void HOURMenu(void){
    count = 0;
    ST7735_FillRect(1, 44, 126, 18, ACC2COLOR);
    ST7735_DrawString(0, 5, "     SELECT HOUR    ", TXTCOLOR);

    //lARGE MIDDLE TEXT
    ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
    ST7735_DrawString2(4, 9, "0", TXTCOLOR);


    //BOTTOM STATUS 1-7
    ST7735_FillRect(75, 149, 17, 10, TXTCOLOR); //2 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

void scrollMinute(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 60){
            count = 0;
        }
        if(count < 0){
            count = 60;
        }

        char date1[5];
        sprintf(date1, "%d", count);
        //lARGE MIDDLE TEXT
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(4, 9, date1, TXTCOLOR);


    }
    RTCmin = count;
}

void MINUTEMenu(void){
    count =0;
    ST7735_FillRect(1, 44, 126, 18, ACC2COLOR);
    ST7735_DrawString(0, 5, "     SELECT MINUTES    ", TXTCOLOR);

    //lARGE MIDDLE TEXT
    ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
    ST7735_DrawString2(4, 9, "0", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(94, 149, 17, 10, TXTCOLOR); //2 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

void scrollSec(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 60){
            count = 0;
        }
        if(count < 0){
            count = 60;
        }

        char date1[5];
        sprintf(date1, "%d", count);
        //lARGE MIDDLE TEXT
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(4, 9, date1, TXTCOLOR);


    }
    RTCsec = count;
}

void SECMenu(void){
    count =0;
    ST7735_FillRect(1, 44, 126, 18, ACC2COLOR);
    ST7735_DrawString(0, 5, "     SELECT SECONDS    ", TXTCOLOR);

    //lARGE MIDDLE TEXT
    ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
    ST7735_DrawString2(4, 9, "0", TXTCOLOR);


    //BOTTOM STATUS 1-7
    ST7735_FillRect(113, 149, 17, 10, TXTCOLOR); //2 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}



unsigned char decToHex(unsigned char n){
    unsigned char result;

    result = (n % 10);
    if(n > 10){
        n -= result;
        result += ((n / 10) % 10) * 16;
    }
    return result;
}




void sendTime(void){

    RTCsec = decToHex(RTCsec);
    RTCmin = decToHex(RTCmin);
    RTChour = decToHex(RTChour);
    RTCday = decToHex(RTCday);
    RTCdate = decToHex(RTCdate);
    RTCmonth = decToHex(RTCmonth);
    RTCyear = decToHex(RTCyear);


    unsigned char RTCtime[7] = {RTCsec, RTCmin, RTChour, RTCday, RTCdate, RTCmonth, RTCyear} ;


    RTC_BurstWrite(slaveAddr, 0, 7, RTCtime);

}

void VOLMenu(void){
    count = 5;
    ST7735_DrawBitmap(0, 160 , datetime, 128, 160);

    //              X   Y   W    H
    ST7735_DrawString(1, 2, "Instrument Setup", TXTCOLOR);

    ST7735_DrawString(0, 5, "     SELECT VOL.    ", TXTCOLOR);


    //lARGE MIDDLE TEXT
    ST7735_DrawString2(4, 9, "5", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(1, 149, 16, 10, TXTCOLOR); //1 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

int scrollVol(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 10){
            count = 1;
        }
        if(count < 1){
            count = 10;
        }

        char date1[5];
        sprintf(date1, "%d", count);
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(4, 9, date1, TXTCOLOR);
    }
    double volPercent = count / 10.0;
    dutyCycle = pow(2, 5.64 * volPercent);

    return count;
}

void SCALEMenu(){
    count = 0;
    ST7735_DrawBitmap(0, 160 , datetime, 128, 160);

    //              X   Y   W    H
    ST7735_DrawString(1, 2, "Instrument Setup", TXTCOLOR);

    ST7735_DrawString(0, 5, "     SELECT SCALE.    ", TXTCOLOR);


    //lARGE MIDDLE TEXT
    ST7735_DrawString2(1, 9, scales[0], TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(1, 149, 16, 10, TXTCOLOR); //1 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

int scrollScales(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 3){
            count = 0;
        }
        if(count < 0){
            count = 3;
        }

        ST7735_DrawString2(1, 9,"       ", TXTCOLOR);
        ST7735_DrawString2(1, 9, scales[count], TXTCOLOR);
    }
    return count;
}

void LowOctMenu(void){
    count = 0;
    ST7735_DrawBitmap(0, 160 , datetime, 128, 160);

    //              X   Y   W    H
    ST7735_DrawString(1, 2, "Instrument Setup", TXTCOLOR);

    ST7735_DrawString(0, 5, "     Octave Low:    ", TXTCOLOR);


    //lARGE MIDDLE TEXT
    ST7735_DrawString2(4, 9, "1", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(1, 149, 16, 10, TXTCOLOR); //1 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

int scrollLowOct(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 7){
            count = 0;
        }
        if(count < 0){
            count = 7;
        }

        char date1[5];
        sprintf(date1, "%d", count + 1);
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(4, 9, date1, TXTCOLOR);
    }
    return count;
}

void HighOctMenu(void){
    count = 7;
    ST7735_DrawBitmap(0, 160 , datetime, 128, 160);

    //              X   Y   W    H
    ST7735_DrawString(1, 2, "Instrument Setup", TXTCOLOR);

    ST7735_DrawString(0, 5, "     Octave High:    ", TXTCOLOR);


    //lARGE MIDDLE TEXT
    ST7735_DrawString2(4, 9, "8", TXTCOLOR);

    //BOTTOM STATUS 1-7
    ST7735_FillRect(1, 149, 16, 10, TXTCOLOR); //1 IS COLORED
    ST7735_DrawString(0, 15, " 1  2  3  4  5  6  7  ", TXTCOLOR);
}

int scrollHighOct(void){
    if(CLK_FLAG) {
        CLK_FLAG = 0;
        //Get Output from CLK and DT, return 1 = CW; 0 = CCW
        int clkOutput;
        clkOutput = compareClocks();
        if(clkOutput)
            count += 1;
        else
            count -= 1;

        if(count > 7){
            count = 0;
        }
        if(count < 0){
            count = 7;
        }

        char date1[5];
        sprintf(date1, "%d", count + 1);
        ST7735_DrawString2(1, 9, "        ", TXTCOLOR);
        ST7735_DrawString2(4, 9, date1, TXTCOLOR);
    }
    return count;
}
