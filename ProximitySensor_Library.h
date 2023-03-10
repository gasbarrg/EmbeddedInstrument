/*
 * ProximitySensor_Library.h
 *
 *  Created on: Oct 8, 2022
 *      Author: Gabe
 */

#ifndef PROXIMITYSENSOR_LIBRARY_H_
#define PROXIMITYSENSOR_LIBRARY_H_

/// ***************| FUNCTIONS | ***********************//*
void prox_init(void);
void getDistance(void);
double getPercentDistance(int max);



/// ***************| VARIABLES | ***********************//*
volatile int toggle;
volatile int count;
volatile int countReady;


#endif /* PROXIMITYSENSOR_LIBRARY_H_ */
