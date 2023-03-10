/*
 * Encoder.h
 *
 *  Created on: Sep 23, 2022
 *      Author: gasbarrg
 */

#ifndef ENCODER_LIBRARY_H_
#define ENCODER_LIBRARY_H_

void Encoder_Init(void);
int Read_DT(void);
int compareClocks(void);
uint8_t encoderSelect(void);
void waitSW(void);

//LCD

void resetSquares(void);
void scrollYear(void);
void YEARMenu(void);
void scrollMonth(void);
void MONTHMenu(void);
void scrollDate2(void);
void DATEMenu(void);
void scrollDay(void);
void DAYMenu(void);
void scrollHour(void);
void HOURMenu(void);
void scrollMinute(void);
void MINUTEMenu(void);
void scrollSec(void);
void SECMenu(void);

void sendTime(void);
int scrollVol(void);
void VOLMenu(void);
void SCALEMenu();
int scrollScales(void);
void LowOctMenu();
int scrollLowOct(void);
void HighOctMenu();
int scrollHighOct(void);

const extern char *scales[7];
extern int rtcTimeout, timeoutCount, inMenu, segAnim;
volatile int CLK_FLAG;
volatile int SW_FLAG;
volatile int currState;
volatile int lastState;
volatile int _encoderReady, _scrollReady, count, lstCount, year, clkOutput;


/********************* Macro Prototypes ********************************
***********************************************************************/
#define BGCOLOR     0x3AC7  //0x53EA  //Green BG
#define ACC1COLOR   0xD6B8  //Light Gray BG
#define ACC2COLOR   0x39E6  //Dark Gray BG
#define TXTCOLOR    0xA62A  //Green Text



#endif /* ENCODER_LIBRARY_H_ */
