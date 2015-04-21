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

/** \file vstimer.c
 * Implementation of virtual system timers
 * (Реализация виртуальных системных таймеров).
 */

#include "bitmask.h"
#include "ce_errors.h"
#include "smcontrol.h"

// For 10 ms interval
#define TIMER2_RELOAD_VALUE  0
#define DIVIDER_RELOAD       4

volatile s_timer8_t  force_measure_timeout_counter = 0;   //!< used by measuring process when engine is stopped
volatile s_timer8_t  ce_control_time_counter = CE_CONTROL_STATE_TIME_VALUE; //!< used for counting of time intervals for CE
volatile s_timer8_t  engine_rotation_timeout_counter = 0; //!< used to determine that engine was stopped
volatile s_timer16_t  bc_status_counter = 0;

uint8_t divider = DIVIDER_RELOAD;

volatile uint16_t sys_counter = 0;                        //!< system tick counter, 1 tick = 10ms

extern volatile uint16_t sm_steps;
extern volatile uint8_t sm_flags;
extern uint16_t sm_steps_b;
extern volatile uint16_t sm_steps_cnt;

/**Interrupt routine which called when T/C 2 overflovs - used for counting time intervals in system
 *(for generic usage). Called each 2ms. System tick is 10ms, and so we divide frequency by 5
 */
ISR(TIMER0_OVF_vect)
{
 TCNT0 = TIMER2_RELOAD_VALUE;

 _ENABLE_INTERRUPT();
 
  s_timer_update_inc(hall_timer);
  if (s_timer_is_action(hall_timer)) hall_timer_ovf_cb(); 
 if (!CHECKBIT(sm_flags,SM_PULSE_STATE) && CHECKBIT(sm_flags,SM_LATCH)) {
  sm_steps_b = sm_steps;
  WRITEBIT (sm_flags,SM_LATCH,0);
 }

 if (sm_steps_b)
 {
  if (!CHECKBIT(sm_flags,SM_PULSE_STATE)) {
	stpmot_stp_set (1);
    WRITEBIT(sm_flags,SM_PULSE_STATE,1);
  }
  else {//The step occurs on the rising edge of ~CLOCK signal
   stpmot_stp_set (0);
   WRITEBIT(sm_flags,SM_PULSE_STATE,0);
   --sm_steps_b;
   ++sm_steps_cnt; //count processed steps
  }
 }

 if (divider > 0)
  --divider;
 else
 {//each 10 ms
  divider = DIVIDER_RELOAD;
  s_timer_update(force_measure_timeout_counter);
  s_timer_update(ce_control_time_counter);
  s_timer_update(engine_rotation_timeout_counter);
  s_timer_update(bc_status_counter);
  ++sys_counter;
 }
}

void s_timer_init(void)
{
 TCCR0B|= _BV(CS01) | _BV(CS00); //clock = 125 kHz & 8 MHz RC
 TCNT0 = 0;
 TIMSK|= _BV(TOIE0);
}

uint8_t s_timer_get (){
	return TCNT0;
}