/*
 * LCD_Librar.h
 *
 *  Created on: Nov 4, 2022
 *      Author: Gabe
 */

#ifndef LCD_LIBRARY_H_
#define LCD_LIBRARY_H_

extern char date[];

void getDate(void);
void scrollDate();
void getTemp(void);


volatile extern int scrollCount;

#endif /* LCD_LIBRARY_H_ */
