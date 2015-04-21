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

/** \file starter.c
 * Implementation of working with starter
 * (Реализация работы со стартером).
 */

#include "ecudata.h"
#include "compilopt.h"

void starter_control(struct ecudata_t* d)
{
 //control of starter's blocking (starter is blocked after reaching the specified RPM, but will not turn back!)
 //управление блокировкой стартера (стартер блокируется после достижения указанных оборотов, но обратно не включается!)
 if (d->sens.frequen > PARAM_STARTER_OFF)
  d->st_block = 1;
 if (d->sens.frequen < 30)
  d->st_block = 0; //unblock starter (снимаем блокировку стартера)
}
