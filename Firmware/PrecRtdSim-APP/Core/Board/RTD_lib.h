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

/* Basic function to set output resistance of the RTD emulator */
void set_switch_rezistor(uint32_t request);

/* Operating modes */
void setResistance(void);   // direct resistance mode
void setNTC(float temp);    // NTC simulation
void setPT(float temp);     // Platinum RTD simulation

/* Slew-rate mode */
void tempSlewRate(uint8_t rtd_mode);
void tempSlewRateSetMin(void);


#endif /* INC_RTD_LIB_H_ */
