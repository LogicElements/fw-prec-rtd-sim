/*
 * MAX7300.c
 *
 *  Created on: 19. 10. 2023
 *      Author: Evzen Steif
 */


#include "MAX7300.h"
#include "RTD_lib.h"
#include "common.h"
#include "i2c.h"

/**
* @brief: Initialization MAX7300 values
* @note: normal operation mode
*/
	void MAX_Init() {
		uint8_t cmd = 0x01;
		HAL_I2C_Mem_Write(&hi2c2, MAX_I2C_ADDR, MAX_CONFIG, 1, &cmd, 1, HAL_MAX_DELAY);
	}

/**
* @brief: MAX7400 port configuration as GPIO output
*/
	void MAX_conf(){

		uint8_t addr;
		uint8_t cmd = 0x55;

		uint8_t conf_addr[5] = {MAX_PC1_CONF, MAX_PC2_CONF, MAX_PC3_CONF, MAX_PC4_CONF, MAX_PC5_CONF};

		for (int i = 0; i < 5; i++){
		addr = conf_addr[i];
		HAL_I2C_Mem_Write(&hi2c2, MAX_I2C_ADDR, addr, 1, &cmd, 1, HAL_MAX_DELAY);
		}
	}

/**
* @brief: MAX7400 write GPIO output  based on the input request
* @param: uint32_t input format: 31...................12 pin
* @note: 1 = GPIO active high, 0 = GPIO active low
*/

	void MAX_write_bin(uint32_t b){

		uint8_t cmd = 0x00;
		uint8_t i = 0;
		uint8_t addr = MAX_SET_12;

		for (i = 0; i <= 17; i++){
			cmd = 0x01&(b >> i);
			HAL_I2C_Mem_Write(&hi2c2, MAX_I2C_ADDR, addr, 1, &cmd, 1, HAL_MAX_DELAY);
			addr += 1;
			}
	}



