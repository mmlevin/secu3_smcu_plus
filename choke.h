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

/** \file choke.h
 * Carburetor choke control.
 * (���������� ��������� ��������� �����������).
 */

#ifndef _CHOKE_H_
#define _CHOKE_H_

struct ecudata_t;

/** Initialization of used I/O ports */
void choke_init_ports(void);

/** Initialization of the module (state variables etc)*/
void choke_init(void);

/** Does control of choke (���������� ��������� ���������)
 * \param d pointer to ECU data structure
 */
void choke_control(struct ecudata_t* d);

#endif //_CHOKE_H_
