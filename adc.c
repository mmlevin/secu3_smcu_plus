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

/** \file adc.c
 * Implementation of ADC related functions (API).
 * Functions for read values from ADC, perform conversion to phisical values etc
 * (Функции для работы с АЦП, считывание значений, преобразование в физические величины и т.д.).
 */

#include "adc.h"
#include "compilopt.h"
#include "bitmask.h"
#include "magnitude.h"
#include "ecudata.h"
#include "ce_errors.h"
#include "bc_status.h"

/**номер канала используемого для ДТОЖ */
#define ADCI_TEMP               2
#define ADCI_INT_TEMP           15

/** переменные состояния АЦП */
adcstate_t adc;

uint16_t adc_get_temp_value(void)
{
 uint16_t value;
 _BEGIN_ATOMIC_BLOCK();
 value = adc.temp_value;
 _END_ATOMIC_BLOCK();
 return value;
}

uint16_t adc_get_int_temp_value(void)
{
    uint16_t value;
    _BEGIN_ATOMIC_BLOCK();
    value = adc.int_temp_value;
    _END_ATOMIC_BLOCK();
    return value;
}

void adc_begin_measure(uint8_t speed2x)
{
 //мы не можем запускать новое измерение, если еще не завершилось
 //предыдущее измерение
 if (!adc.sensors_ready)
  return;

 adc_init_ports();
 
 adc.sensors_ready = 0;
 ADMUX = ADCI_INT_TEMP|ADC_VREF_TYPE_INT_TEMP;
 if (speed2x) {
  CLEARBIT(ADCSRA, ADPS1);
  SETBIT(ADCSRA, ADPS0);
 } else {
  CLEARBIT(ADCSRA, ADPS0);
  SETBIT(ADCSRA, ADPS1);
 }
  
 SETBIT(ADCSRA, ADSC);
}

void adc_begin_measure_all(void)
{
 adc.measure_all = 1;
 adc_begin_measure(0); //<--normal speed
}

void adc_init(void)
{
 adc.measure_all = 0;

 //инициализация АЦП, параметры: f = 125.000 kHz @ 8MHz
 //внутренний источник опорного напряжения или внешний зависит от опции VREF_5V, прерывание разрешено
 ADMUX=ADC_VREF_TYPE;
 ADCSRA=_BV(ADEN)|_BV(ADIE)|_BV(ADPS2)|_BV(ADPS1);

 //модуль АЦП готов к новому измерению
 adc.sensors_ready = 1;

 //запрещаем компаратор - он нам не нужен
 ACSR=_BV(ACD);
}

/**прерывание по завершению преобразования АЦП. Измерение значений всех аналоговых датчиков. После запуска
 * измерения это прерывание будет вызываться для каждого входа, до тех пор пока все входы не будут обработаны.
 */
ISR(ADC_vect)
{
 _ENABLE_INTERRUPT();

 switch(ADMUX & 0x0F)
 {
  case ADCI_INT_TEMP:
    adc.int_temp_value = ADC;
    ADMUX = ADCI_TEMP|ADC_VREF_TYPE;
    SETBIT(ADCSRA,ADSC);
  break;     
  case ADCI_TEMP: //закончено измерение температуры
   adc.temp_value = ADC;
   if (0==adc.measure_all)
   {
	   ADMUX = ADCI_INT_TEMP|ADC_VREF_TYPE_INT_TEMP; // Запуск измерения следующего параметра
	   adc.sensors_ready = 1;
	   
	   ce_init_ports();
	   ce_set_state (edat.ce_state | bc.blink_state);
   }
   else
   {
	   adc.measure_all = 0;
       ADMUX = ADCI_INT_TEMP|ADC_VREF_TYPE_INT_TEMP; // Запуск измерения следующего параметра
	   SETBIT(ADCSRA,ADSC);   	   
   }
   break;
 }
}

int16_t adc_compensate(int16_t adcvalue, int16_t factor, int32_t correction)
{
 return (((((int32_t)adcvalue*factor)+correction)<<2)>>16);
}

//Coolant sensor has linear output. 10mV per C (e.g. LM235)
int16_t temp_adc_to_c(int16_t adcvalue)
{
 if (adcvalue < 0)
  adcvalue = 0;
 return (adcvalue - ((int16_t)((TSENS_ZERO_POINT / ADC_DISCRETE)+0.5)) );
}

//Coolant sensor has linear output. 10mV per C (e.g. LM235)
int16_t int_temp_adc_to_c(int16_t adcvalue)
{
    if (adcvalue < 0)
    adcvalue = 0;
    return (TEMP_PHYSICAL_MAGNITUDE_MULTIPLIER*(adcvalue - ((int16_t)INTERNAL_TEMPERATURE_OFFSET)));
}
