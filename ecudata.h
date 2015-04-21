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

/** \file ecudata.h
 * ECU data in RAM (global data structures and state variables).
 * This file contains main data structures used in the firmware.
 */

#ifndef _ECUDATA_H_
#define _ECUDATA_H_

#include "tables.h"

/**Describes all system's inputs - theirs derivative and integral magnitudes
 * описывает все входы системы - их производные и интегральные величины
 */
typedef struct sensors_t
{
 int16_t  temperat;                      //!< Coolant temperature (температура охлаждающей жидкости (усредненная))
 int16_t  int_temperat;                  //!< IC temperature
 uint16_t frequen;                       //!< Averaged RPM (частота вращения коленвала (усредненная))
 uint16_t inst_frq;                      //!< Instant RPM - not averaged  (мгновенная частота вращения)
 uint8_t  carb;                          //!< State of carburetor's limit switch (состояние концевика карбюратора)
 int16_t  temperat_raw;                  //!< raw ADC value from coolant temperature sensor
 int16_t  int_temperat_raw;              //!< raw ADC value from internal temperature sensor
}sensors_t;

/**Describes system's data (main ECU data structure)
 * описывает данные системы, обеспечивает единый интерфейс данных
 */
typedef struct ecudata_t
{
 struct sensors_t sens;                  //!< --sensors (сенсоры)

 uint8_t  st_block;                      //!< State of the starter blocking output (состояние выхода блокировки стартера)
 uint8_t  ce_state;                      //!< State of CE lamp (состояние лампы "CE")
 uint8_t  choke_pos;                     //!< Choke position in % * 2

  uint8_t choke_rpm_reg;                  //!< Used to indicate that at the moment system regulates RPM by means of choke position
}ecudata_t;

extern struct ecudata_t edat;            //!< ECU data structure. Contains all related data and state information

/**Initialization of variables and data structures
 * \param d pointer to ECU data structure
 */
void init_ecu_data(void);

#endif //_ECUDATA_H_
