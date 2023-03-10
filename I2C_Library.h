/*
 * I2C_Library.h
 *
 *  Created on: Sep 30, 2022
 *      Author: gasbarrg
 */

#ifndef I2C_LIBRARY_H_
#define I2C_LIBRARY_H_

volatile int number, bin;

void RTC_init(void);
void writinghour(void);
void writingminutes(void);
void writingseconds(void);
void writingday(void);
void writingdate(void);
void writingmonth(void);
void writingyear(void);
int RTC_Write(int slaveAddr, unsigned char memAddr, unsigned char data);
int RTC_Read(int slaveAddr, unsigned char memAddr, unsigned char* data);
void RTC_BurstWrite(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data);
int RTC_BurstRead(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data);
float RTC_TempRead();

extern unsigned char dateTime[];
extern unsigned char dateTimeReturn[];
const extern char *days[];
const extern char *months[];
extern int slaveAddr;

#endif /* I2C_LIBRARY_H_ */
