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

/** \file funconv.h
 * Core mathematics and regulation logic.
 * (Основная часть математического аппарата и логики регулирования).
 */

#ifndef _FUNCONV_H_
#define _FUNCONV_H_

#include <stdint.h>
#include "vstimer.h"
#include "compilopt.h"

/** f(x) liniar interpolation for function with single argument
 * \param x argument value
 * \param a1 function value at the beginning of interval
 * \param a2 function value at the end of interval
 * \param x_s argument value at the beginning of interval
 * \param x_l length of interval in x
 * \param m function multiplier
 * \return interpolated value of function * m
 */
int16_t simple_interpolation(int16_t x,int16_t a1,int16_t a2,int16_t x_s,int16_t x_l, uint8_t m);

struct ecudata_t;

/** Restricts specified value to specified limits
 * \param io_value pointer to value to be restricted. This parameter will also receive result.
 * \param i_bottom_limit bottom limit
 * \param i_top_limit upper limit
 */
void restrict_value_to(int16_t *io_value, int16_t i_bottom_limit, int16_t i_top_limit);

/**Converts ADC value into phisical magnitude - temperature (given from thermistor)
 * (переводит значение АЦП в физическую величину - температура для резистивного датчика (термистор))
 * \param adcvalue Voltage from sensor (напряжение с датчика - значение в дискретах АЦП))
 * \return физическая величина * TEMP_PHYSICAL_MAGNITUDE_MULTIPLAYER
 */
int16_t thermistor_lookup(uint16_t adcvalue);

/** Obtains choke position (closing %) from coolant temperature using lookup table
 * Получает положение воздушной заслонки (% закрытия) по температре охлаждающей жидкости
 * \param d pointer to ECU data structure
 * \param p_prev_temp pointer to state variable used to store temperature value between calls of
 * this function
 * \return choke closing percentage (value * 2)
 */
uint8_t choke_closing_lookup(struct ecudata_t* d, int16_t* p_prev_temp);

/**Initialization of regulator's data structures*/
void chokerpm_regulator_init(void);

/** RPM regulator function for choke position
 * \param d pointer to ECU data structure
 * \param p_prev_corr pointer to state variable used to store calculated correction between calls of
 * this function
 * \return choke closing correction in SM steps
 */
int16_t choke_rpm_regulator(struct ecudata_t* d, int16_t* p_prev_corr);

#endif //_FUNCONV_H_
