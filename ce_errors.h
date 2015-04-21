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

/** \file ce_errors.h
 * Control of CE, errors detection and related functionality.
 * (���������� ������ CE, ����������� ������ � ��������������� ����������������).
 */

#ifndef _CE_ERRORS_H_
#define _CE_ERRORS_H_

#include "vstimer.h"

#define CE_CONTROL_STATE_TIME_VALUE    50  //!< used for CE (flashing)

//define bits (numbers of bits) of errors (Check Engine)
//(���������� ���� (������ �����) ������ (Check Engine)).
#define ECUERROR_CKPS_MALFUNCTION       0  //!< CKP sensor malfunction
#define ECUERROR_EEPROM_PARAM_BROKEN    1  //!< Parameters in EEPROM are broken. CE turns off after a few seconds after the engine starts (�E �������� ����� ��������� ������ ����� ������� ���������)
#define ECUERROR_PROGRAM_CODE_BROKEN    2  //!< FLASH is broken. CE turns off after a few seconds after the engine starts (�� �������� ����� ��������� ������ ����� ������� ���������)
#define ECUERROR_KSP_CHIP_FAILED        3  //!< Knock detection chip does not work properly
#define ECUERROR_KNOCK_DETECTED         4  //!< Knock was detected (one or many times)
#define ECUERROR_MAP_SENSOR_FAIL        5  //!< MAP sensor does not work
#define ECUERROR_TEMP_SENSOR_FAIL       6  //!< Coolant temperature sensor does not work
#define ECUERROR_VOLT_SENSOR_FAIL       7  //!< Voltage is wrong or sensing is disconnected
#define ECUERROR_DWELL_CONTROL          8  //!< Problems with dwell control (overcharge etc)
#define ECUERROR_CAMS_MALFUNCTION       9  //!< CAM sensor malfunction
#define ECUERROR_TPS_SENSOR_FAIL       10  //!< TPS sensor does not work

struct ecudata_t;

/**checks for errors and manages the CE lamp
 * (���������� �������� ������� ������ � ��������� ������ CE).
 * \param d pointer to ECU data structure
 * \param ce_control_time_counter time counter object
 */
void ce_check_engine(struct ecudata_t* d, volatile s_timer8_t* ce_control_time_counter);

/**Set specified error (number of bit)
 * (��������� ��������� ������ (����� ����)).
 * \param error code of error
 */
void ce_set_error(uint8_t error);

/**Reset specified error (number of bit)
 * (C���� ��������� ������ (����� ����)).
 * \param error code of error
 */
void ce_clear_error(uint8_t error);

/**Initialization of used I/O ports (������������� ������������ ������ �����/������). */
#define ce_init_ports() {WRITEBIT (DDRB,PB4,1)}

/**Turns on/off CE lamp (��������/��������� ����� Check Engine).*/
#define ce_set_state(s) {WRITEBIT(PORTB,PB4,s)}

#endif //_CE_ERRORS_H_
