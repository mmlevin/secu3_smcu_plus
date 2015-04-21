/* SECU-3 SMCU PLUS - An open source, free choke control unit
   Copyright (C) 2015 Maksim M. Levin. Russia, Voronezh
 
   SECU-3  - An open source, free engine control unit
   Copyright (C) 2007 Alexey A. Shabelnikov. Ukraine, Kiev

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   contacts:
              http://secu-3.org
              email: shabelnikov@secu-3.org
*/

/** \file compilopt.h
 * Definitions of compile-time options (preprocessor directives).
 */

#ifndef _COMPILOPT_H_
#define _COMPILOPT_H_

#define THERMISTOR_CS (1)                                               // Coolant sensor is thermistor
#define PARAM_HALL_SELEDGE_P (0)                                        // Active hall sensor edge. 0 is falling, 1 is rising
#define PARAM_ENGINE_CYL_NUMBER (4)                                     // Number of engine's cylynders
#define PARAM_CARB_INVERS (0)                                           // Carburettor limit switch inverse flag
#define PARAM_STARTER_OFF (600)                                         //  RPM when starter turning off (used for engine works detection)
#define PARAM_IDLREG_TURN_ON_TEMP (200)                                 // Temperature of disabling idle RPM regulator
#define PARAM_CTS_USE_MAP (1)                                           // Use lookup table for temperature sensor
#define PARAM_TEMP_ADC_FACTOR (ADC_COMP_FACTOR(ADC_VREF_FACTOR) - 900)  // Temperature correction factor for ADC adjustment
#define PARAM_TEMP_ADC_CORRECTION (ADC_COMP_CORR(ADC_VREF_FACTOR, 0.0)) // Temperature correction coefficient for ADC adjustment
#define PARAM_CHOKE_CORR_TEMP (40)                                      // Temperature threshold for choke startup correction
#define PARAM_CHOKE_CORR_TIME (300)                                     // Time for choke startup correction
#define PARAM_CHOKE_RPM_IF (51)                                         // Choke RPM regulator integral factor (*1024)
#define PARAM_CHOKE_STARTUP_CORR (20)                                   // Startup correction value for choke
#define PARAM_CHOKE_SM_STEPS (800)                                      // Number of steps in choke stepper motor
#define PARAM_CHOKE_RPM_0 (2000)                                        //  Value of starting RPM value while warm up
#define PARAM_CHOKE_RPM_1 (1200)                                        // Value of finish RPM value while warm up
#define PARAM_CTS_VL_BEGIN (ROUND(0.182 / ADC_DISCRETE))                // Beginning voltage of themperature lookup table
#define PARAM_CTS_VL_END (ROUND(4.25 / ADC_DISCRETE))                   // End voltage of themperature lookup table
#define INTERNAL_TEMPERATURE_OFFSET (270)                               // Offset value for internal temperature sensor

#define PARAM_RC_CALIBRATION_FREQ   (7920)                              // RC calibration frequency @ 20 celsius degrees
#define PARAM_RC_CORRECTION_BEGIN_TEMP (-40)                    // Begin RC calibration temperature
#define PARAM_RC_CORRECTION_END_TEMP (80)                       // End RC calibration temperature
#define PARAM_RC_CORRECTION_BEGIN_FREQ (7730)                           // Begin RC calibration frequency @ 5.0V
#define PARAM_RC_CORRECTION_END_FREQ (8094)                             // End RC calibration frequency @ 5.0V


#endif //_COMPILOPT_H_
