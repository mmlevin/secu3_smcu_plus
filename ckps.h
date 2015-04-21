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

/** \file ckps.h
 * Processing of crankshaft position sensor.
 * (Обработка датчика положения коленвала).
 */

#ifndef _CKPS_H_
#define _CKPS_H_

/**Initialization of CKP module (hardware & variables)
 * (инициализирет структуру данных/состояния ДПКВ и железо на которое он мапится)
 */
void ckps_init_state(void);

/** Calculate instant RPM using last measured period
 * (Высчитывание мгновенной частоты вращения коленвала основываясь на последнем измеренном значении периода)
 * \return RPM (min-1)
 */
uint16_t ckps_calculate_instant_freq(void);

/**\return 1 if there was engine stroke and reset flag!
 * (эта функция возвращает 1 если был новый такт зажигания и сразу сбрасывает событие!)
 * \details Used to perform synchronization with rotation of crankshaft.
 */
uint8_t ckps_is_stroke_event_r(void);

/** Enable/disable spark generation using shutter entering (used on startup - at low RPM)
 * Note: This function is applicable only when synchronization from Hall sensor is selected
 * \param i_shutter 1 - use shutter, 0 - don't use shutter (use timer)
 */
void ckps_set_shutter_spark(uint8_t i_shutter);


/** Initialization of state variables */
void ckps_init_state_variables(void);

/** \return returns 1 if number of current tooth has been changed
 *  (возвращает 1, если номер текущего зуба изменился)
 */
uint8_t ckps_is_cog_changed(void);

/** Initialization of used I/O ports (производит инициализацию линий портов) */
void ckps_init_ports(void);

#endif //_CKPS_H_
