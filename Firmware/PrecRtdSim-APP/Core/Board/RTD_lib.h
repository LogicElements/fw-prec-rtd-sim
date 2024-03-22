/*
 * RTD_lib.h
 *
 *  Created on: 16. 10. 2023
 *      Author: evzen
 */

#include "common.h"

#ifndef INC_RTD_LIB_H_
#define INC_RTD_LIB_H_


void RTD_Init();
void RTD_Handle();
void switch_position(uint32_t request, uint8_t temp_corr, float temperature, float temp_koef);
void set_switch_rezistor(uint32_t request, uint8_t temp_corr, float temp_koef);
void rezistorArrayTemperature(float temperature, float temp_koef);
void checkSlewRateModeChange();
void tempSlewRate(uint8_t rtd_mode, uint8_t temp_corr);
void tempSlewRateSetMin();


void setResistance(uint8_t temp_corr);
void setPT(uint8_t temp_corr, float temp);
void setNTC (uint8_t temp_corr, float temp);


#endif /* INC_RTD_LIB_H_ */
