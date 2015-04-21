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

/** \file crc16.h
 * CRC16 related functions.
 * Functions for calculate CRC16 of data in RAM and in the ROM
 * (������� ��� ���������� ����������� ����� ��� ������ � ��� � � ���).
 */

#ifndef _CRC16_H_
#define _CRC16_H_

#include "port/pgmspace.h"

/** Calculates CRC16 for given block of data in RAM
 * (��������� ����������� ����� CRC16 ��� ����� ������ � ���).
 * \param buf pointer to block of data (RAM) (��������� �� �������� �����)
 * \param num size of block to process (������ ������ � ������)
 * \return calculated CRC16 (����������� ����� CRC16)
 */
uint16_t crc16(uint8_t *buf, uint16_t num);

/** Calculates CRC16 for given block of data in ROM
 * (��������� ����������� ����� CRC16 ��� ����� ������ � ���).
 * \param buf pointer to block of data (ROM) (��������� �� �������� �����)
 * \param num size of block to process (������ ������ � ������)
 * \return calculated CRC16 (����������� ����� CRC16)
 */
uint16_t crc16f(uint8_t _PGM *buf, uint16_t num);

/** Calculates CRC8 for given byte using given seed (previous CRC value)
 * The polynomial is X8 + X5 + X4 + 1 (1-Wire bus)
 * \param data Byte which CRC will be calculated
 * \param crc Previous CRC value or zero
 * \return calculated value of CRC8
 */
uint8_t update_crc8(uint8_t data, uint8_t crc);

#endif //_CRC16_H_
