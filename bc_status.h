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
 * Blink codes status definition
 */

#ifndef BC_STATUS_H_
#define BC_STATUS_H_

#include <stdint.h>

#define BC_STATUS_INIT_TIMEOUT 400 // 4 sec timeout
#define BC_STATUS_TIMEOUT 30 // 300 msec timeout

typedef struct bc_status_t
{
	uint8_t state;
	uint8_t digits;
	uint8_t data;
	uint8_t counter;
	uint8_t blink_state;
} bc_status_t;

extern struct bc_status_t bc;

void bc_status_init();
void bc_status (void);

#endif /* BC_STATUS_H_ */