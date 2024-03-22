/*
 * RTD_lib.c
 *
 *  Created on: 16. 10. 2023
 *      Author: Evzen Steif
 */


#include "RTD_lib.h"
#include "MAX7300.h"
#include "configuration.h"
#include "SHT20.h"


/* Private constants ---------------------------------------------------------*/
#define tick_second 1000
/* Private variables ---------------------------------------------------------*/

/**
 * Instance of all private variables
 */

typedef struct
{
	uint32_t switch_on_resistance;
	uint32_t switch_position[6];
	uint32_t resistor_array[6];
	uint32_t resistor_array_temp_calib[6];

	float resistance;
	float temperature;
	float tempSR;

	float temp_c;
	float temp_koef;

	uint32_t tick;
	uint8_t config;

}RTD_t;


static RTD_t rtd_app;


/**
* @brief: Initialization RTD values
*/
	void RTD_Init(){
		// values of resistor array - implicitly soldered to the board (mΩ)

	    rtd_app.resistor_array[0] = 1.2e3;
	    rtd_app.resistor_array[1] = 9.53e3;
	    rtd_app.resistor_array[2] = 75e3;
	    rtd_app.resistor_array[3] = 590e3;
	    rtd_app.resistor_array[4] = 4700e3;
	    rtd_app.resistor_array[5] = 37400e3;

		// temperature coefficient of resistor array in ppm/K
		rtd_app.temp_koef = 100;

		// average analog switch on resistance of NX3L4051PW (mΩ)
		rtd_app.switch_on_resistance = 0.36e3;

		rtd_app.tempSR =  conf.rtd.slewrate_min;
		rtd_app.config = 1;
	}



/* Auxiliary function */


	/**
	* @brief: exponential polynomial approximation
	*/

	float exponential(float x, int n)
	{
		float result = 1.0;
		float term = 1.0;

		for (int i = 1; i <= n; ++i) {
			term *= x / i;
			result += term;
		}
		return result;
	}



	/**
	* @brief: Calculation of resistor array with temperature dependence
	* @param 		* temperature format: float - in celsius
	                * temp koef format: float   - in ppm/K
	*/

	void rezistorArrayTemperature(float temperature, float temp_koef)
	{
		int8_t i = 0;
		// loop through each element in the resistor array
		for (i = 0; i < 6; i++) {
			// adjust each resistor value based on the temperature and temperature coefficient
			rtd_app.resistor_array_temp_calib[i] = rtd_app.resistor_array[i] *(1 + (temperature - 20.0) * temp_koef / 1000000);
		}
	}


/* Basic functions of RTD */


		/**
		* @brief: Calculation of resistor array switch positions according to the request
		* @param request: uint32_t, from 10 to 290000 in Ω
		*        temp_corr: uint8_t, 0 = temp correction OFF, 1 = temp correction ON
		*        temperature: float, temperature near the resistor decade in °C
		*        temp_koef: float, temperature coefficient of resistor array in ppm/K
		*/

	void switch_position(uint32_t request, uint8_t temp_corr, float temperature, float temp_koef)
	{
	    uint8_t multiple = 0;
	    int8_t i = 5;
	    float resistance = rtd_app.switch_on_resistance * 6;
	    request *= 1000;  // convert request to miliohms

	    // If temperature correction is enabled, perform correction of resistors in the array
	    if (temp_corr) {
	        rezistorArrayTemperature(temperature, temp_koef);
	    }

	    // until the requirement is reached
	    while (i >= 0 && resistance < request) {

	    	// calculate how many times a given resistor needs to be added
	    	multiple = (request - resistance)/((temp_corr) ? rtd_app.resistor_array_temp_calib[i] : rtd_app.resistor_array[i]);
	    	rtd_app.switch_position[i] = multiple;  // store the switch position in the array

	    	// limit the maximum number of resistors in a row of decades
	        if (multiple >= 7)
	        {
	            multiple = 7;
	        }

	        // recalculate the total resistance in the system considering the added resistors
	        resistance += multiple * (temp_corr ? rtd_app.resistor_array_temp_calib[i] : rtd_app.resistor_array[i]);
	        i--;  // move to the next resistor in the array
	    }
	}


		/**
		* @brief: Set the electrical resistance at the output of the RTD emulator according to the request
		* @param request: uint32_t, from 10 to 290000 in Ω
		* 	     temp_corr: uint8_t, 0 = temp correction OFF, 1 = temp correction ON
		* 		 temp_koef: float, temperature coefficient of resistor array in ppm/K
		*/

	void set_switch_rezistor(uint32_t request, uint8_t temp_corr, float temp_koef)
	{
		int8_t i = 0;

		// if temperature correction is enabled, read current temperature from sensor, otherwise set to 0
		rtd_app.temp_c = temp_corr ? SHT20_GetTemperature() : 0;

		// call the function to set the switch position based on the given parameters
		switch_position(request, temp_corr, rtd_app.temp_c, temp_koef);

		uint32_t cmd = 0x00;

		for (i = 0; i < 6; i++)
		{
		cmd = cmd | rtd_app.switch_position[i] << i * 3; //combine switch positions into the command
		}

		MAX_write_bin(cmd); //write the command to the I2C expander module
	}


/* Handle functions */


	/**
	* @brief: Direct adjustment of the resistance value according to the request
	* @param: temp_corr: uint8_t, 0 = temp correction OFF, 1 = temp correction ON
	* @note: the requirement is based on modbus communication (RTD_RESISTANCE)
	* 		 called when RTD_MODE = 0
	*/

	void setResistance(uint8_t temp_corr)
	{

		// check if the resistance value is within the valid range
		if (conf.rtd.resistance >= 10 && conf.rtd.resistance <= 290000)
		{
	    // call the function to set the switch resistor
		set_switch_rezistor(conf.rtd.resistance,temp_corr, rtd_app.temp_koef);
		}
	}

	/**
	* @brief: Setting the desired temperature of the simulated NTC
	* @param: temp_corr: uint8_t, 0 = temp correction OFF, 1 = temp correction ON
	* 		  temp: float, simulated temperature requirement
	* @note: the requirement is based on modbus communication (RTD_TEMPERATURE)
	*        called when RTD_MODE = 1
	*/

	void setNTC (uint8_t temp_corr, float temp)
	{

		// set the temperature value to the provided value if non-zero, otherwise use the global value
		rtd_app.temperature = (temp)? temp:conf.rtd.temperature;

		// check if the temperature is within the valid range
		if (rtd_app.temperature >= -30 && rtd_app.temperature<=200)
		{
			// calculate the value based on the NTC beta value and the difference between the temperature and the reference temperature
			float val = conf.rtd.ntc_beta*(1.0/(rtd_app.temperature+273.15) - 1.0/(25.0+273.15));
			rtd_app.resistance = conf.rtd.ntc_stock_res*exponential(val, 20);

			// call the function to set the switch resistor based on the calculated resistance value
			set_switch_rezistor(rtd_app.resistance,temp_corr, rtd_app.temp_koef);

	    }
	}

	/**
	* @brief: Setting the desired temperature of the simulated platinum RTD
	* @param: temp_corr: uint8_t, 0 = temp correction OFF, 1 = temp correction ON
	* 		  temp: float, simulated temperature requirement
	* @note: the requirement is based on modbus communication (RTD_TEMPERATURE)
	* 		 called when RTD_MODE = 2
	*/

	void setPT(uint8_t temp_corr, float temp)
	{

		// set the temperature value to the provided value if non-zero, otherwise use the global value
		rtd_app.temperature = (temp)? temp:conf.rtd.temperature;

		// check if the temperature is within the valid range
		if(rtd_app.temperature >= -30 && rtd_app.temperature<=200)
		{
		float A = 3.91e-3;  // define the constant A for PT calculation
		// calculate the resistance based on the PT stock resistance and the linear temperature relationship
		rtd_app.resistance =  conf.rtd.pt_stock_res*(1+A*rtd_app.temperature);
		// call the function to set the switch resistor based on the calculated resistance value
		set_switch_rezistor(rtd_app.resistance,temp_corr, rtd_app.temp_koef);
		}

	}


	/**
	* @brief: Continuous increase of simulated temperature on request
	* @param: rtd_mode: uint8_t, 0 = NTC, 1 = Platinum RTD
	* 		  temp_corr: uint8_t, 0 = temp correction OFF, 1 = temp correction ON
	* @note: the requirement is based on modbus communication
	* 		 called when  RTD_SLEWRATE_MODE = 1 and NTC or Platinum RTD is selected (RTD_MODE)
	*        RTD_SLEWRATE in °C/s - the temperature by which the request increases every second
	*		 RTD_SLEWRATE_MIN - start of simulated temperatures
	*		 RTD_SLEWRATE_MAX - end of simulated temperatures
	*/

	void tempSlewRate(uint8_t rtd_mode, uint8_t temp_corr)
	{

		// if configuration is set, update tick and configuration flags
	    if (rtd_app.config) {
	        rtd_app.tick = HAL_GetTick() + tick_second;
	        rtd_app.config = 0;
	    }

	    // if tick is set and has expired - the function is called every second
	    if (rtd_app.tick != 0 && TICK_EXPIRED(rtd_app.tick)) {

	        rtd_app.tempSR += conf.rtd.slewrate;
	        rtd_app.tick = 0;
	        rtd_app.config = 1;

	        // ensure temperature slew rate does not exceed configured maximum
	        rtd_app.tempSR = (rtd_app.tempSR >= conf.rtd.slewrate_max) ? conf.rtd.slewrate_max : rtd_app.tempSR;

	        // call appropriate function based on RTD mode (PT or NTC)
	        (rtd_mode) ? setPT(temp_corr, rtd_app.tempSR) : setNTC(temp_corr, rtd_app.tempSR);
	    }

	}

	void tempSlewRateSetMin()
	{
		rtd_app.tempSR = conf.rtd.slewrate_min;
	}








