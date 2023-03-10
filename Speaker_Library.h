/**************************************************************************************
*                                Speaker_Libary.h
*                                Gabriel Gasbarre
*                                 October 20022
*
*     This is a library for control of an external speaker using Timer A on the MSP423
* **************************************************************************************/
#ifndef SPEAKER_LIBRARY_H_
#define SPEAKER_LIBRARY_H_


/// ***************| FUNCTIONS | ***********************//*
void Speaker_init();
void Clock_Init48MHz(void);
void speakerOn(void);
void speakerOff(void);
void funkyTown(void);
void funkyTown2(void);
void setHz(double Hz);
void setDutCycle(int percent);
void playNote(double note);
void playScale(const int Scale[], int max, int min, int size);
void LEDnotes(int percent);

/// ***************| VARIABLES | ***********************//*
static const int
    C1, CS1, D1, DS1, E1, F1, FS1, G1, GS1, A1, AS1, B1,
    C2, CS2, D2, DS2, E2, F2, FS2, G2, GS2, A2, AS2, B2,
    C3, CS3, D3, DS3, E3, F3, FS3, G3, GS3, A3, AS3, B3,
    C4, CS4, D4, DS4, E4, F4, FS4, G4, GS4, A4, AS4, B4,
    C5, CS5, D5, DS5, E5, F5, FS5, G5, GS5, A5, AS5, B5,
    C6, CS6, D6, DS6, E6, F6, FS6, G6, GS6, A6, AS6, B6,
    C7, CS7, D7, DS7, E7, F7, FS7, G7, GS7, A7, AS7, B7,
    C8, CS8, D8, DS8, E8, F8, FS8, G8, GS8, A8, AS8, B8;

#define NOTES 96
extern int notes[NOTES];

#define BLUES 48
extern int Blues[BLUES];

#define C_MAJOR 56
extern int C_Major[C_MAJOR];

#define C_MINOR 56
extern int C_Minor[C_MINOR];

#define MAJOR_PENTATONIC 40
extern int Major_Pentatonic[MAJOR_PENTATONIC];


extern int maxDistance;
extern int dutyCycle;
volatile int speakerToggle;
#endif /* SPEAKER_LIBRARY_H_ */
