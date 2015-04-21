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

/** \file funconv.c
 * Implementation of core mathematics and regulation logic.
 * (–еализаци€ основной части математического аппарата и логики регулировани€).
 */

#include <stdlib.h>
#include "ecudata.h"
#include "magnitude.h"
#include "compilopt.h"

// ‘ункци€ линейной интерпол€ции
// x - значение аргумента интерполируемой функции
// a1,a2 - значени€ функции в узлах интерпол€ции
// x_s - значение аргумента функции в начальной точке
// x_l - длина отрезка(проекции на ось х) между точками
// m - function multiplier
// возвращает интерполированное значение функции * m
int16_t simple_interpolation(int16_t x, int16_t a1, int16_t a2, int16_t x_s, int16_t x_l, uint8_t m)
{
 return ((a1 * m) + (((int32_t)(a2 - a1) * m) * (x - x_s)) / x_l);
}

//ограничивает указанное значение указанными пределами
void restrict_value_to(int16_t *io_value, int16_t i_bottom_limit, int16_t i_top_limit)
{
 if (*io_value > i_top_limit)
  *io_value = i_top_limit;
 if (*io_value < i_bottom_limit)
  *io_value = i_bottom_limit;
}

//Coolant sensor is thermistor (тип датчика температуры - термистор)
//Note: We assume that voltage on the input of ADC depend on thermistor's resistance linearly.
//Voltage on the input of ADC can be calculated as following (divider resistors are used):
// U3=U1*Rt*R2/(Rp(Rt+R1+R2)+Rt(R1+R2));
// Rt - thermistor, Rp - pulls up thermistor to voltage U1,
// R1,R2 - voltage divider resistors.
int16_t thermistor_lookup(uint16_t adcvalue)
{
 int16_t i, i1;

 //Voltage value at the start of axis in ADC discretes (значение напр€жени€ в начале оси в дискретах ј÷ѕ)
 uint16_t v_start = PARAM_CTS_VL_BEGIN;
 //Voltage value at the end of axis in ADC discretes (значение напр€жени€ в конце оси в дискретах ј÷ѕ)
 uint16_t v_end = PARAM_CTS_VL_END;

 uint16_t v_step = (v_end - v_start) / (THERMISTOR_LOOKUP_TABLE_SIZE - 1);

 if (adcvalue < v_start)
  adcvalue = v_start;

 i = (adcvalue - v_start) / v_step;

 if (i >= THERMISTOR_LOOKUP_TABLE_SIZE-1) i = i1 = THERMISTOR_LOOKUP_TABLE_SIZE-1;
 else i1 = i + 1;

 return (simple_interpolation(adcvalue, (int16_t)PGM_GET_WORD(&fw_data.exdata.cts_curve[i]), (int16_t)PGM_GET_WORD(&fw_data.exdata.cts_curve[i1]), //<--values in table are signed
        (i * v_step) + v_start, v_step, 16)) >> 4;
}

uint8_t choke_closing_lookup(struct ecudata_t* d, int16_t* p_prev_temp)
{
 int16_t i, i1, t = d->sens.temperat;

 //if difference between current and previous temperature values is less than +/-0.5,
 //then previous value will be used for calculations.
 if (abs(*p_prev_temp - t) < TEMPERATURE_MAGNITUDE(0.5))
  t = *p_prev_temp;
 else
  *p_prev_temp = t; //make it current

 //-5 - минимальное значение температуры
 if (t < TEMPERATURE_MAGNITUDE(-5))
  t = TEMPERATURE_MAGNITUDE(-5);

 //5 - шаг между узлами интерпол€ции по температуре
 i = (t - TEMPERATURE_MAGNITUDE(-5)) / TEMPERATURE_MAGNITUDE(5);

 if (i >= 15) i = i1 = 15;
 else i1 = i + 1;

 if ((t > TEMPERATURE_MAGNITUDE(70)) || (0==PGM_GET_BYTE(&fw_data.exdata.choke_closing[i1])))
  return 0;
 else
 {
  uint8_t pos = simple_interpolation(t, PGM_GET_BYTE(&fw_data.exdata.choke_closing[i]), PGM_GET_BYTE(&fw_data.exdata.choke_closing[i1]),
  (i * TEMPERATURE_MAGNITUDE(5)) + TEMPERATURE_MAGNITUDE(-5), TEMPERATURE_MAGNITUDE(5), 16) >> 4;
  return (pos==1) ? 0 : pos; //0.5% is same as zero
 }
}

/**Describes state data for idling regulator */
typedef struct
{
 int16_t int_state;   //!< regulator's memory (integrated error)
}chokeregul_state_t;

/**Variable. State data for choke RPM regulator */
chokeregul_state_t choke_regstate;

//reset of choke RPM regulator state
void chokerpm_regulator_init(void)
{
 choke_regstate.int_state = 0;
}

int16_t choke_rpm_regulator(struct ecudata_t* d, int16_t* p_prev_corr)
{
 int16_t error, rpm, t = d->sens.temperat;

 if (0==PARAM_CHOKE_RPM_0)
 {
  *p_prev_corr = 0;
  return 0; //regulator is turned off, return zero correction
 }

 //-5 - значение температуры cоответствующее оборотам в первой точке
 //70 - значение температуры соответствующее оборотам во второй точке
 restrict_value_to(&t, TEMPERATURE_MAGNITUDE(-5), TEMPERATURE_MAGNITUDE(70));

 //calculate target RPM value for regulator
 rpm = simple_interpolation(t, PARAM_CHOKE_RPM_0, PARAM_CHOKE_RPM_1,
 TEMPERATURE_MAGNITUDE(-5), TEMPERATURE_MAGNITUDE(75), 4) >> 2;

 error = rpm - d->sens.frequen;
 if (abs(error) <= 50)   //dead band is +/-50 RPM
  return *p_prev_corr;

 choke_regstate.int_state+= error >> 2; //update integrator's state
 restrict_value_to(&choke_regstate.int_state, -28000, 28000); //restrict integrаtor output

 *p_prev_corr = (((int32_t)PARAM_CHOKE_RPM_IF) * choke_regstate.int_state) >> 12; //additional 4 shift bits to reduce regulator's influence
 if (0)
 {
  #define _PROPFACT(x) ((int16_t)(x * 8))
  (*p_prev_corr)+= (error * _PROPFACT(0.5)) >> 3; //proportional part
 }
 restrict_value_to(p_prev_corr, -PARAM_CHOKE_SM_STEPS,PARAM_CHOKE_SM_STEPS); //range must be: +/- d->param.sm_steps

 return *p_prev_corr;
}
