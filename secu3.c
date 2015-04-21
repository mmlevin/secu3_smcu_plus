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

/** \file secu3.c
 * Implementation of main module of the firmware.
 * (Реализация главного модуля прошивки).
 */

#include "ce_errors.h"
#include "choke.h"
#include "ckps.h"
#include "crc16.h"
#include "ecudata.h"
#include "magnitude.h"
#include "measure.h"
#include "starter.h"

#include "bc_status.h"
#include "wdt.h"

#define FORCE_MEASURE_TIMEOUT_VALUE   20    //!< timeout value used to perform measurements when engine is stopped
#define ENGINE_ROTATION_TIMEOUT_VALUE 150   //!< timeout value used to determine that engine is stopped (used for Hall sensor)

/**Control of certain units of engine (управление отдельными узлами двигателя).
 * \param d pointer to ECU data structure
 */
void control_engine_units(struct ecudata_t *d)
{
 starter_control(d);
 choke_control(d);
}

/** Check firmware integrity (CRC) and set error indication if code or data is damaged
 */
void check_firmware_integrity(void)
{
 if (crc16f(0, CODE_SIZE)!=PGM_GET_WORD(CODE_SIZE))
  ce_set_error(ECUERROR_PROGRAM_CODE_BROKEN);
}

/**Initialization of I/O ports
 */
void init_ports(void)
{
 ckps_init_ports();
 choke_init_ports();
 meas_init_ports();
}

/**Initialization of system modules
 */
void init_modules(void)
{
 //Initialization of ADC
 adc_init();

 choke_init();

 //инициализируем модуль ДПКВ
 ckps_init_state();

 s_timer_init();

  //проводим несколько циклов измерения датчиков для инициализации данных
 meas_initial_measure(&edat);
 
 bc_status_init();
}


/**Main function of firmware - entry point. Contains initialization and main loop 
 * (Главная функция в коде прошивки. С нее начинается выполнение программы, она содержит
 * код инициализации и главный цикл)
 */
MAIN()
{
 uint8_t turnout_low_priority_errors_counter = 255;
 
 //подготовка структуры данных переменных состояния системы
 init_ecu_data();

 //Perform I/O ports configuration/initialization (конфигурируем порты ввода/вывода)
 init_ports();

 //If firmware code is damaged then turn on CE (если код программы испорчен - зажигаем СЕ)
 check_firmware_integrity();

 //Start watchdog timer! (запускаем сторожевой таймер)
// wdt_start_timer();

 //perform initialization of all system modules
 init_modules();

 //Enable all interrupts globally before we fall in main loop
 //разрешаем глобально прерывания перед запуском основного цикла программы
 _ENABLE_INTERRUPT();

 //------------------------------------------------------------------------
 while(1)
 {
  if (ckps_is_cog_changed())
   s_timer_set(engine_rotation_timeout_counter, ENGINE_ROTATION_TIMEOUT_VALUE);

  if (s_timer_is_action(engine_rotation_timeout_counter))
  { //двигатель остановился (его обороты ниже критических)
   ckps_init_state_variables();
   meas_update_values_buffers(&edat, 1);  //<-- update RPM only
  }

  //запускаем измерения АЦП, через равные промежутки времени. При обнаружении каждого рабочего
  //такта этот таймер переинициализируется. Таким образом, когда частота вращения двигателя превысит
  //определенную величину, то это условие перестанет выполняться.
  if (s_timer_is_action(force_measure_timeout_counter))
  {
   adc_begin_measure_all(); //normal speed
   s_timer_set(force_measure_timeout_counter, FORCE_MEASURE_TIMEOUT_VALUE);
   meas_update_values_buffers(&edat, 0);
  }

  //----------непрерывное выполнение-----------------------------------------
  //управление фиксированием и индицированием возникающих ошибок
  ce_check_engine(&edat, &ce_control_time_counter);
  bc_status();
  //расчет мгновенной частоты вращения коленвала
  edat.sens.inst_frq = ckps_calculate_instant_freq();
  //усреднение физических величин хранящихся в кольцевых буферах
  meas_average_measured_values(&edat);
  //cчитываем дискретные входы системы и переключаем тип топлива
  meas_take_discrete_inputs(&edat);
  //управление периферией
  control_engine_units(&edat);  
  //------------------------------------------------------------------------

  //выполняем операции которые необходимо выполнять строго для каждого рабочего такта.
  if (ckps_is_stroke_event_r())
  {
   meas_update_values_buffers(&edat, 0);
   s_timer_set(force_measure_timeout_counter, FORCE_MEASURE_TIMEOUT_VALUE);
   
   ckps_set_shutter_spark (edat.sens.frequen < 200);

   // индицирование этих ошибок прекращаем при начале вращения двигателя
   //(при прошествии N-го количества тактов)
   if (turnout_low_priority_errors_counter == 1)
   {
    ce_clear_error(ECUERROR_EEPROM_PARAM_BROKEN);
    ce_clear_error(ECUERROR_PROGRAM_CODE_BROKEN);
   }
   if (turnout_low_priority_errors_counter > 0)
    turnout_low_priority_errors_counter--;
  }

  wdt_reset_timer();
 }//main loop
 //------------------------------------------------------------------------
}
