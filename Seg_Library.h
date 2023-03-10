/*
 * 7Seg_Library.h
 *
 *  Created on: Dec 4, 2022
 *      Author: gasbarrg
 */

#ifndef SEG_LIBRARY_H_
#define SEG_LIBRARY_H_


void SegInit(void);
void sendCommand(unsigned char adress, unsigned char data);
void displayDistance(double dist);
void displayHz(int hz);
void animateSeg();
void resetSeg();
#endif /* SEG_LIBRARY_H_ */
