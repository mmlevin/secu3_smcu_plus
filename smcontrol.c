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

/** \file smcontrol.c
 * Implementation of stepper motor control
 * (Реализация управления шаговым двигателем).
 */

#include "port/interrupt.h"
#include "port/intrinsic.h"
#include "smcontrol.h"
#include "bitmask.h"

volatile uint16_t sm_steps = 0;
volatile uint8_t sm_flags = 0;
uint16_t sm_steps_b = 0;
volatile uint16_t sm_steps_cnt = 0;

void stpmot_init_ports(void)
{
 WRITEBIT(PORTB,PB0,SM_DIR_CW);
 WRITEBIT (DDRB,PB0,1);
 WRITEBIT(PORTB,PB3,1);
 WRITEBIT (DDRB,PB3,1);
}

void stpmot_run(uint16_t steps)
{
 if (steps)
 {
  _DISABLE_INTERRUPT();
  sm_steps_cnt = 0;
  _ENABLE_INTERRUPT();
 }
 _DISABLE_INTERRUPT();
  sm_steps = steps;
  WRITEBIT(sm_flags,SM_LATCH,1);
 _ENABLE_INTERRUPT();
}

uint8_t stpmot_is_busy(void)
{
 uint16_t current;
 uint8_t latching;
 _DISABLE_INTERRUPT();
 current = sm_steps_b;
 latching = CHECKBIT(sm_flags,SM_LATCH);
 _ENABLE_INTERRUPT();
 return (current > 0 || latching); //busy?
}

uint16_t stpmot_stpcnt(void)
{
 uint16_t count;
 _DISABLE_INTERRUPT();
 count = sm_steps_cnt;
 _ENABLE_INTERRUPT();
 return count;
}
