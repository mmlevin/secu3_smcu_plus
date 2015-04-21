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

/** \file ce_errors.c
 * Implementation of controling of CE, errors detection and related functionality.
 * (Реализация управления лампой CE, обнаружения ошибок и соответствующей функциональности).
 */

#include "bitmask.h"
#include "ce_errors.h"
#include "ecudata.h"
#include "magnitude.h"
#include "compilopt.h"

/**CE state variables structure */
typedef struct
{
 uint16_t ecuerrors;         //!< 16 error codes maximum (максимум 16 кодов ошибок)
}ce_state_t;

/**State variables */
ce_state_t ce_state = {0};

//operations under errors (операции над ошибками)
/*#pragma inline*/
void ce_set_error(uint8_t error)
{
 SETBIT(ce_state.ecuerrors, error);
}

/*#pragma inline*/
void ce_clear_error(uint8_t error)
{
 CLEARBIT(ce_state.ecuerrors, error);
}

/** Internal function. Contains checking logic */
void check(struct ecudata_t* d)
{
 //checking coolant temperature sensor
if (!THERMISTOR_CS)
{
  // error if (2.28v > voltage > 3.93v)
  if (d->sens.temperat_raw < ROUND(2.28 / ADC_DISCRETE) || d->sens.temperat_raw > ROUND(3.93 / ADC_DISCRETE))
   ce_set_error(ECUERROR_TEMP_SENSOR_FAIL);
  else
   ce_clear_error(ECUERROR_TEMP_SENSOR_FAIL);
}   
else {
  if (!PARAM_CTS_USE_MAP) //use linear sensor
  {
   if (d->sens.temperat_raw < ROUND(2.28 / ADC_DISCRETE) || d->sens.temperat_raw > ROUND(3.93 / ADC_DISCRETE))
    ce_set_error(ECUERROR_TEMP_SENSOR_FAIL);
   else
    ce_clear_error(ECUERROR_TEMP_SENSOR_FAIL);
  }
  else
  {
   // error if (0.2v > voltage > 4.7v) for thermistor
   if (d->sens.temperat_raw < ROUND(0.2 / ADC_DISCRETE) || d->sens.temperat_raw > ROUND(4.7 / ADC_DISCRETE))
    ce_set_error(ECUERROR_TEMP_SENSOR_FAIL);
   else
    ce_clear_error(ECUERROR_TEMP_SENSOR_FAIL);
  }
 }
}

//If any error occurs, the CE is light up for a fixed time. If the problem persists (eg corrupted the program code),
//then the CE will be turned on continuously. At the start of program CE lights up for 0.5 seconds. for indicating
//of the operability.
//При возникновении любой ошибки, СЕ загорается на фиксированное время. Если ошибка не исчезает (например испорчен код программы),
//то CE будет гореть непрерывно. При запуске программы СЕ загорается на 0.5 сек. для индицирования работоспособности.
void ce_check_engine(struct ecudata_t* d, volatile s_timer8_t* ce_control_time_counter)
{
 check(d);

 //If the timer counted the time, then turn off the CE
 //если таймер отсчитал время, то гасим СЕ
 if (s_timer_is_action(*ce_control_time_counter))
 {
  _BEGIN_ATOMIC_BLOCK();
  d->ce_state = 0; //<--doubling
  _END_ATOMIC_BLOCK();
 }

 //If at least one error is present  - turn on CE and start timer
 //если есть хотя бы одна ошибка - зажигаем СЕ и запускаем таймер
 if (ce_state.ecuerrors!=0)
 {
  s_timer_set(*ce_control_time_counter, CE_CONTROL_STATE_TIME_VALUE);
    _BEGIN_ATOMIC_BLOCK();
  d->ce_state = 1;  //<--doubling
  _END_ATOMIC_BLOCK();
 }
}
