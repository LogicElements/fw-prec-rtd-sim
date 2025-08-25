/*
 * PCA9505.h
 *
 *  Created on: Jul 31, 2025
 *      Author: evzen
 */

#ifndef INC_PCA9505_H_
#define INC_PCA9505_H_

#include "stm32f0xx_hal.h"
#include "reg_map.h"
#include <stdint.h>

typedef struct {
    uint8_t addr8_E1;
    uint8_t addr8_E2;
    uint8_t has_E1;
    uint8_t has_E2;
} PCA_Module;

extern PCA_Module g_B1;
extern PCA_Module g_B2;


typedef enum { PCA9505_PORT0=0, PCA9505_PORT1=1, PCA9505_PORT2=2, PCA9505_PORT3=3, PCA9505_PORT4=4 } PCA9505_Port;

uint8_t pca_addr_from_a(uint8_t a2a1a0);
HAL_StatusTypeDef PCA_InitDevice(I2C_HandleTypeDef *hi2c, uint8_t addr8);
HAL_StatusTypeDef PCA_PinWrite(I2C_HandleTypeDef *hi2c, uint8_t addr8, PCA9505_Port port, uint8_t bit, uint8_t state);
void PCA_WriteCmd18(I2C_HandleTypeDef *hi2c, uint8_t addr8, uint32_t cmd18, uint8_t channel_is_A);
HAL_StatusTypeDef PCA_InitFromConf(I2C_HandleTypeDef *hi2c);
void PCA_WriteCmd18_Module(I2C_HandleTypeDef *hi2c, uint8_t module_idx /*1=B1,2=B2*/, uint32_t cmd18, uint8_t channel_is_A);
void PCA_BlinkLed(I2C_HandleTypeDef *hi2c, PCA_Module *m,uint8_t use_E1, uint8_t channel_is_A);

#endif
