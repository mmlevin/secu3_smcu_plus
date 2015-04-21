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

/** \file adc.h
 * ADC related functions (API).
 * Functions for read values from ADC, perform conversion to phisical values etc
 * (Функции для работы с АЦП, считывание значений, преобразование в физические величины и т.д.).
 */

#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

/**одна дискрета АЦП в вольтах (ADC discrete in Volts)*/
#define ADC_DISCRETE            0.0025

/**наклон прямой датчика температуры вольт/градус */
#define TSENS_SLOPP             0.01

/**напряжение на выходе датчика температуры при 0 градусов цельсия */
#define TSENS_ZERO_POINT        2.73

//константа для выбора источника опорного напряжения и коэфф. компенсации опорного напряжения
#define ADC_VREF_TYPE          0x90    //!< Vref selection constant for 2.56V
#define ADC_VREF_TYPE_INT_TEMP 0x80    //!< Vref selection constant for 1.1V
#define ADC_VREF_FACTOR        1.0000  //!< Vref compensation factor (2.56V/2.56V)

/**Cтруктура данных состояния АЦП */
typedef struct
{
    volatile uint16_t temp_value;   //!< последнее измеренное значение температуры охлаждающей жидкости
    volatile uint16_t int_temp_value;//!< значение измерения температуры микропроцессора
    volatile uint8_t sensors_ready; //!< датчики обработаны и значения готовы к считыванию
    uint8_t  measure_all;           //!< если 1, то производится измерение всех значений
}adcstate_t;

extern adcstate_t adc;

#define adc_init_ports() {WRITEBIT (DDRB,PB4,0); WRITEBIT (PORTB,PB4,0);}

/** Получение последнего измеренного значения с ДТОЖ
 * \return значение в дискретах АЦП
 */
uint16_t adc_get_temp_value(void);

/** Получение последнего измеренного значения с датчика температуры контроллера
 * \return значение в дискретах АЦП
 */
uint16_t adc_get_int_temp_value(void);

/**запускает измерение значений с датчиков, но только если предыдущее
 * измерение завершено.
 * \param speed2x Double ADC clock (0,1) (Удвоение тактовой частоты АЦП)
 */
void adc_begin_measure(uint8_t speed2x);


/**запускает измерение значений с датчиков и сигнала с ДД. Первыми снимаются значения
 * с датчиков, последним сигнал с ДД
 */
void adc_begin_measure_all(void);

/**проверка готовности АЦП
 *\return возвращает не 0 если измерение готово (АЦП не занято)
 */
#define adc_is_measure_ready() (adc.sensors_ready)

/**инициализация АЦП и его переменных состояния */
void adc_init(void);

/**компенсация погрешностей АЦП или входных цепей (погрешность смещения и передаточная погрешность)
 * \param adcvalue значение АЦП для компенсации
 * \param factor коэффициен масштабирования
 * \param correction смещение
 * \return compensated value (компенсированное значение)
 * \details
 * factor = 2^14 * gainfactor,
 * correction = 2^14 * (0.5 - offset * gainfactor),
 * 2^16 * realvalue = 2^2 * (adcvalue * factor + correction)
 */
int16_t adc_compensate(int16_t adcvalue, int16_t factor, int32_t correction);

/**Converts ADC value into phisical magnituge - temperature (given from linear sensor)
 * Переводит значение АЦП в физическую величину - температура, для линейного датчика
 * \param adcvalue Voltage from sensor (напряжение с датчика - значение в дискретах АЦП)
 * \return физическая величина * TEMP_PHYSICAL_MAGNITUDE_MULTIPLAYER
 */
int16_t temp_adc_to_c(int16_t adcvalue);

/**Converts ADC value into phisical magnituge - temperature (given from linear sensor)
 * Переводит значение АЦП в физическую величину - температура, для линейного датчика
 * \param adcvalue Voltage from sensor (напряжение с датчика - значение в дискретах АЦП)
 * \return физическая величина * TEMP_PHYSICAL_MAGNITUDE_MULTIPLAYER
 */
int16_t int_temp_adc_to_c(int16_t adcvalue);

#endif //_ADC_H_
