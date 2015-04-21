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

/** \file tables.c
 * Tables and datastructures stored in the frmware (instances).
 * (“аблицы и структуры данных хранимые в прошивке).
 */

#include "magnitude.h"
#include "tables.h"

/**Fill whole firmware data */
PGM_DECLARE (fw_data_t fw_data) =
{
 /**ƒополнительные данные по умолчанию Fill additional data with default values */
 {
  /**Fill coolant temperature sensor lookup table*/
  {_TLV(120.0), _TLV(95.0), _TLV(79.0), _TLV(66.5), _TLV(57.4), _TLV(49.5), _TLV(43.8), _TLV(37.9),
   _TLV(31.0), _TLV(24.8), _TLV(19.8), _TLV(13.8), _TLV(6.0), _TLV(-1.0), _TLV(-12.5), _TLV(-30.0)},

  /**Fill choke closing vs. temperarure lookup table*/
  {_CLV(100.0), _CLV(99.0), _CLV(98.0), _CLV(96.0), _CLV(95.0), _CLV(92.0), _CLV(86.0), _CLV(78.0),
   _CLV(69.0),  _CLV(56.0), _CLV(50.0), _CLV(40.0), _CLV(25.0), _CLV(12.0), _CLV(5.0),  _CLV(0)}},
};
