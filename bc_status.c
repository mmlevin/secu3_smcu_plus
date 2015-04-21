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
              email: mmlevin@mail.ru
*/

/** \file bc_status.h
 * Blink codes status logic
 */

#include "bc_status.h"
#include "vstimer.h"
#include "magnitude.h"
#include "ecudata.h"
#include <stdlib.h>

bc_status_t bc;

void bc_status_init() {
	bc.state = 0;
	bc.digits = 0;
	bc.data = 0;
	bc.counter = 0;
	bc.blink_state = 0;
}

void bc_load_data () {
	bc.data = abs(edat.sens.temperat / TEMP_PHYSICAL_MAGNITUDE_MULTIPLIER);
	if (bc.data >= 100) bc.digits = 3;
	else if (bc.data >= 10) bc.digits = 2;
	else bc.digits = 1;
	if (edat.sens.temperat < -4) ++bc.digits;
}

void bc_process_digit () {
    uint8_t cur_digit = 0;
	if (bc.digits) {
		switch (bc.digits) {
			case 3:
				cur_digit = bc.data / 100;
				break;
			case 2:
				cur_digit = (bc.data % 100) / 10;
				break;
			case 1:
				cur_digit = bc.data % 10;
				break;
			default:
				break;
		}
		bc.counter = cur_digit?cur_digit:10;
		bc.digits--;
	}
}

void bc_status(void) 
{
	uint8_t bstate=0;
	if (s_timer_is_action(bc_status_counter)) {
		switch (bc.state) {
			case 0: // Init pause
				bc.state = 1;
				s_timer_set(bc_status_counter,BC_STATUS_INIT_TIMEOUT);	
				break;
			case 1: // Intercharacter delay
				bc_load_data();
				bc.state = 2;			
				s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT*3);
				break;
			case 2: // Process digit
				bc_process_digit();			
				bstate = 1;		
				if (bc.counter == 10) {
					bc.state = 4; // Process zero digit
					bc.counter = 1;
					s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT*6);
				} else {
					bc.state = 4; // Process non-zero digit
					s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT);
				}
				break;
			case 3:
				bc.state = 4; // Process current digit
				bstate = 1;
				s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT);			
				break;
			case 4:
				bstate = 0;
				if (--bc.counter) {
					bc.state = 3; // Still processing current digit
					s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT);
				} else {
					bc.state = 5; // Interdigit delay
					s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT);
				}
				s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT);			
				break;
			case 5: // Interdigit delay
				if (bc.digits) { // If not all digits indicated
					bc.state = 2;
					s_timer_set(bc_status_counter,BC_STATUS_TIMEOUT*2);
					break;
				} else {
					bc.state = 1;
					s_timer_set(bc_status_counter,BC_STATUS_INIT_TIMEOUT);
					break;
				}		
				break;
		}
		_BEGIN_ATOMIC_BLOCK();
		bc.blink_state = bstate;
		_END_ATOMIC_BLOCK();
	}
}
