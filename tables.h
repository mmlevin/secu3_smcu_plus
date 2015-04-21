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

#ifndef _TABLES_H_
#define _TABLES_H_

#include "port/pgmspace.h"

/**For specifying of choke position in the lookup table*/
#define _CLV(v) ROUND(((v)*2.0))

/**For specifying of temperature values in the lookup table*/
#define _TLV(v) ROUND(((v)*4.0))

#define THERMISTOR_LOOKUP_TABLE_SIZE    16          //!< Size of lookup table for coolant temperature sensor
#define CHOKE_CLOSING_LOOKUP_TABLE_SIZE 16          //!< Size of lookup table defining choke closing versus coolant temperature

/**Describes separate tables stored in the firmware
 */
typedef struct fw_ex_data_t
{
  /**Coolant temperature sensor lookup table 
   * (таблица значений температуры с шагом по напряжению) */
  int16_t cts_curve[THERMISTOR_LOOKUP_TABLE_SIZE];

  /**Choke closing versus coolant temperature */
  uint8_t choke_closing[CHOKE_CLOSING_LOOKUP_TABLE_SIZE];
}fw_ex_data_t;


/**Описывает все данные находящиеся в прошивке 
 * Describes all data residing in firmware */
typedef struct fw_data_t
{
 fw_ex_data_t exdata;                    //!< Additional data containing separate tables
}fw_data_t;

/**Размер переменной контрольной суммы прошивки в байтах
 * Size of variable of CRC of whole firmware in bytes */
#define CODE_CRC_SIZE  sizeof(uint16_t)

/**Размер секции приложения без учета контрольной суммы
 * Size of application's section without taking into account its CRC */
#define CODE_SIZE (((unsigned int)FLASHEND)-CODE_CRC_SIZE+1)

//================================================================================
//Variables:

/**Все данные прошивки All firmware data */
PGM_DECLARE (extern fw_data_t fw_data);

#endif //_TABLES_H_
