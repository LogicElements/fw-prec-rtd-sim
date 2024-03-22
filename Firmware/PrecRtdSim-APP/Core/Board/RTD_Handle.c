/*
 * RTD_Handle.c
 *
 *  Created on: 15. 2. 2024
 *      Author: evzen
 */


#include "RTD_Handle.h"
#include "RTD_lib.h"
#include "configuration.h"



void RTD_Handle(){


	// if slew rate mode is disabled
	if(!conf.rtd.slewrate_mode)
	{

		tempSlewRateSetMin();   // set temperature slew rate to minimum value

		 // choose action based on RTD mode
		switch(conf.rtd.mode){

		case 0: // direct resistance mode
			setResistance(conf.rtd.temp_correction);
			break;
		case 1:  // NTC mode
			setNTC(conf.rtd.temp_correction,0);
			break;
		case 2:  // PT mode
			setPT(conf.rtd.temp_correction,0);
			break;
		default: //default action (set direct resistance without temp correction)
			setResistance(0);
			break;
							 }
	}


	else // if slew rate mode is enabled
	{
		// choose action based on RTD mode
		switch (conf.rtd.mode) {

		case 1: // NTC mode with temperature slew rate
			tempSlewRate(0,conf.rtd.temp_correction);
			break;
		case 2: // PT mode with temperature slew rate
			tempSlewRate(1,conf.rtd.temp_correction);
			break;
		default: //default action (set direct resistance without temp correction)
			setResistance(0);
		    break;
								}
	}
}




