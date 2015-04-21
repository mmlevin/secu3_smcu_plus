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

/** \file ecudata.c
 * ECU data in RAM (global data structures and state variables)
 */

#include "ecudata.h"

/**ECU data structure. Contains all related data and state information */
struct ecudata_t edat;


/**Initialization of variables and data structures
 * \param d pointer to ECU data structure
 */
void init_ecu_data(void)
{
 edat.sens.inst_frq = 0;
 edat.ce_state = 0;
 edat.st_block = 0; //starter is not blocked
  edat.choke_pos = 0;
  edat.choke_rpm_reg = 0;
}
