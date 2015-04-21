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

/** \file hall.c
 * Implementation of Hall sensor's synchronization processing.
 * (Реализация обработки синхронизации от датчика Холла).
 */

#include "bitmask.h"
#include "magnitude.h"
#include "tables.h"     //fnptr_t
#include "vstimer.h"
#include "compilopt.h"

/**Maximum number of ignition channels (cylinders) */
#define IGN_CHANNELS_MAX      8

//Flags (see flags variable)
#define F_VHTPER    1                 //!< used to indicate that measured period is valid (actually measured)
#define F_STROKE    2                 //!< flag for synchronization with rotation (флаг синхронизации с вращением)
#define F_HALLEV    3                 //!< flag indicates presence of Hall sensor event
#define F_SPSIGN    4                 //!< Sign of the measured stroke period (time between TDCs)
#define F_SHUTTER   5                 //!< indicates using of shutter entering for spark generation (used at startup)
#define F_SHUTTER_S 6                 //!< synchronized value of F_SHUTTER

/** State variables */
typedef struct
{
 uint16_t measure_start_value;        //!< previous value if timer 1 used for calculation of stroke period
 volatile uint16_t stroke_period;     //!< stores the last measurement of 1 stoke (Хранит последнее измерение периода такта двигателя)
 volatile uint8_t t1oc;               //!< Timer 1 overflow counter
 volatile uint8_t t1oc_s;             //!< Contains value of t1oc synchronized with stroke_period value
}hallstate_t;

volatile s_timer8_t hall_timer = 0;
volatile uint8_t hall_flags;

hallstate_t hall;                     //!< instance of state variables

/**Table srtores dividends for calculating of RPM */
#define FRQ_CALC_DIVIDEND(channum) PGM_GET_DWORD(&frq_calc_dividend[channum])
PGM_DECLARE(uint32_t frq_calc_dividend[1+IGN_CHANNELS_MAX]) =
 //     1          2          3          4         5         6         7         8
 {0, 15000000L, 7500000L, 5000000L, 3750000L, 3000000L, 2500000L, 2142857L, 1875000L};

/**Set edge type for PS input*/
#define SET_PS_EDGE(type) {\
  if ((type)) \
   MCUCR = (MCUCR | _BV(ISC01)) & ~_BV(ISC00); \
  else \
   MCUCR|=_BV(ISC01) | _BV(ISC00); }

void ckps_init_state_variables(void)
{
 _BEGIN_ATOMIC_BLOCK();

 hall.stroke_period = 0xFFFF;

 CLEARBIT(hall_flags, F_STROKE);
 CLEARBIT(hall_flags, F_VHTPER);
 CLEARBIT(hall_flags, F_HALLEV);
 CLEARBIT(hall_flags, F_SPSIGN);
 SETBIT(hall_flags,F_SHUTTER);
 SETBIT(hall_flags,F_SHUTTER_S);

 SET_PS_EDGE(PARAM_HALL_SELEDGE_P);  //set previously selected edge for INT1

 hall.t1oc = 0;                       //reset overflow counter
 hall.t1oc_s = 255;                   //RPM is very low

 _END_ATOMIC_BLOCK();
}

void ckps_init_state(void)
{
 _BEGIN_ATOMIC_BLOCK();
 //set flag indicating that Hall sensor input is available
 ckps_init_state_variables();
 
 GIMSK|=  _BV(INT0); //INT0 enabled only when Hall sensor input is available

 _END_ATOMIC_BLOCK();
}

void ckps_init_ports(void)
{
	WRITEBIT (PORTB,PB2,1); // enable pullup
}

//Высчитывание мгновенной частоты вращения коленвала по измеренному периоду между тактами двигателя
//(например для 4-цилиндрового, 4-х тактного это 180 градусов)
//Период в дискретах таймера (одна дискрета = 4мкс), в одной минуте 60 сек, в одной секунде 1000000 мкс.
uint16_t ckps_calculate_instant_freq(void)
{
 uint16_t period; uint8_t ovfcnt, sign;
 //ensure atomic acces to variable (обеспечиваем атомарный доступ к переменной)
 _DISABLE_INTERRUPT();
 period = hall.stroke_period;        //stroke period
 ovfcnt = hall.t1oc_s;               //number of timer overflows
 sign = CHECKBIT(hall_flags, F_SPSIGN);   //sign of stroke period
 _ENABLE_INTERRUPT();

 //We know period and number of timer overflows, so we can calculate correct value of RPM even if RPM is very low
 if (sign && ovfcnt > 0)
  return FRQ_CALC_DIVIDEND(PARAM_ENGINE_CYL_NUMBER) / ((((int32_t)ovfcnt) * 65536) - (65536-period));
 else
  return FRQ_CALC_DIVIDEND(PARAM_ENGINE_CYL_NUMBER) / ((((int32_t)ovfcnt) * 65536) + period);
}

uint8_t ckps_is_stroke_event_r()
{
 uint8_t result;
 _BEGIN_ATOMIC_BLOCK();
 result = CHECKBIT(hall_flags, F_STROKE) > 0;
 CLEARBIT(hall_flags, F_STROKE);
 _END_ATOMIC_BLOCK();
 return result;
}

uint8_t ckps_is_cog_changed(void)
{
 uint8_t result;
 _BEGIN_ATOMIC_BLOCK();
 result = CHECKBIT(hall_flags, F_HALLEV) > 0;
 CLEARBIT(hall_flags, F_HALLEV);
 _END_ATOMIC_BLOCK();
 return result;
}

void ckps_set_shutter_spark(uint8_t i_shutter)
{
 _BEGIN_ATOMIC_BLOCK();
 WRITEBIT(hall_flags, F_SHUTTER, i_shutter);
 _END_ATOMIC_BLOCK();
}


/** Special function for processing falling edge,
 * must be called from ISR
 * \param tmr Timer value at the moment of falling edge
 */
INLINE
void ProcessFallingEdge(uint16_t tmr)
{
 //save period value if it is correct. We need to do it forst of all to have fresh stroke_period value
 if (CHECKBIT(hall_flags, F_VHTPER))
 {
  //calculate stroke period
  hall.stroke_period = tmr - hall.measure_start_value;
  WRITEBIT(hall_flags, F_SPSIGN, tmr < hall.measure_start_value); //save sign
  hall.t1oc_s = hall.t1oc, hall.t1oc = 0; //save value and reset counter
 }
 SETBIT(hall_flags, F_VHTPER);
 SETBIT(hall_flags, F_STROKE); //set the stroke-synchronization event (устанавливаем событие тактовой синхронизации)
 hall.measure_start_value = tmr;
 
 if (!CHECKBIT(hall_flags,F_SHUTTER_S))
 {
	adc_begin_measure(_AB(hall.stroke_period,1) < 4) ;
 }
}

ISR(INT0_vect)
{
 uint16_t tmr = hall_timer << 8 | s_timer_get();
 //toggle edge
 if (MCUCR & _BV(ISC00))
 { //falling
  if (!PARAM_HALL_SELEDGE_P)  
   ProcessFallingEdge(tmr);
   MCUCR&= ~(_BV(ISC00));  //next edge will be rising
 }
 else
 { //rising
  if (PARAM_HALL_SELEDGE_P)
   ProcessFallingEdge(tmr);
  MCUCR|=_BV(ISC00); //next will be falling
 }
 WRITEBIT(hall_flags,F_SHUTTER_S,CHECKBIT(hall_flags,F_SHUTTER));
 SETBIT(hall_flags, F_HALLEV); //set event flag
}

/** Timer 1 overflow interrupt.
 * Used to count timer 1 overflows to obtain correct revolution period at very low RPMs (10...400 min-1)
 */
void hall_timer_ovf_cb(void)
{
 ++hall.t1oc;
}
