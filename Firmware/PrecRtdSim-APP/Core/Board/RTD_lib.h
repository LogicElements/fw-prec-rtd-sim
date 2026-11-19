/*
 * RTD_lib.h
 *
 *  Created on: 16. 10. 2023
 *      Author: evzen
 */

#include "common.h"

#ifndef INC_RTD_LIB_H_
#define INC_RTD_LIB_H_


void RTD_Init(void);

/* Basic function to set output resistance of an RTD channel */
void set_switch_rezistor(uint16_t channel, uint32_t request);

/* Operating modes on the channel of RTD_CHANNEL_SELECT */
void setResistance(void);   // direct resistance mode
void setNTC(float temp);    // NTC simulation
void setPT(float temp);     // Platinum RTD simulation

/* Temperature of one channel - NTC (pt = 0) or platinum RTD (pt = 1) */
void RTD_SetChannelTemperature(uint16_t channel, uint8_t pt, float temp);

/* Slew-rate mode */
void tempSlewRate(uint8_t rtd_mode);
void tempSlewRateSetMin(void);


#endif /* INC_RTD_LIB_H_ */
