/*
 * LCD_Library.c
 *
 *  Created on: Nov 4, 2022
 *      Author: Gabe
 */

#include "I2C_Library.h"
#include "msp.h"
#include "ST7735.h"
#include "Encoder_Library.h"
#include <stdio.h>
#include <string.h>

extern char date[65] = "";
volatile extern int scrollCount = 1;


void getDate(void){
    //Read Date from RTC
    RTC_BurstRead(slaveAddr, 0, 7, dateTimeReturn);
    //Remove Additional bits from HRS
    dateTimeReturn[2] &=~ BIT6;
    //Remove Additional bits from HRS
    dateTimeReturn[2] &=~ BIT5;
    //Remove Century Bit from Months:
    dateTimeReturn[5] &=~ BIT7;


    sprintf(date, "           %s, %s %x, 20%02x At %02x:%02x:%02x              ",
           days[dateTimeReturn[3] -1], months[dateTimeReturn[5]-1],
           dateTimeReturn[4], dateTimeReturn[6],
           dateTimeReturn[2], dateTimeReturn[1], dateTimeReturn[0]);
}

void getTemp(void){
    char temp[5];
    sprintf(temp, "%.2fC", RTC_TempRead());
    ST7735_DrawString(4, 8, temp, TXTCOLOR);      //String to select instrument configuration
}

void scrollDate(){
    if(_scrollReady){
        //Update Time Every Second (scrollCount increments every .2s)
        if(scrollCount % 4 == 0)
            getDate();
        //Update Temp
        if(scrollCount % 10 == 0)
            getTemp();
        _scrollReady = 0;


        //PRINT UP TO 11 CHARACTERS
        int c = 0;
        char sub[11];
        while (c < 12) {
           sub[c] = date[scrollCount+c-1];
           c++;
        }
        sub[c] = '\0';


        ST7735_DrawString2(0, 1, sub , TXTCOLOR);        //Prompts user to select an option

        scrollCount++;
        if(date[scrollCount + 11] == 0){
            scrollCount = 0;
        }
    }

}


