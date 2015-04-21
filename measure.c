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

/** \file measure.c
 * Implementation pf processing (averaging, corrections etc) of data comes from ADC and sensors
 * (Реализация обработки (усреднение, корректировки и т.д.) данных поступающих от АЦП и датчиков).
 */

#include "bitmask.h"
#include "ecudata.h"
#include "funconv.h"    //thermistor_lookup(), ats_lookup
#include "magnitude.h"
#include "compilopt.h"

 /**Special macro for compensating of voltage division (with voltage divider)*/
#define _RESDIV(v, n, d) (((n) * (v)) / (d))

/**Reads state of throttle gate (only the value, without inversion)
 * считывает состояние дроссельной заслонки (только значение, без инверсии)
 */
#define GET_THROTTLE_GATE_STATE() (CHECKBIT(PINB, PINB1) != 0)

/**Number of values for averaging of RPM for tachometer
 * кол-во значений для усреднения частоты вращения к.в. для оборотов тахометра */
#define FRQ_AVERAGING           4
#define TMP_AVERAGING           8                 //!< Number of values for averaging of coolant temperature

uint16_t freq_circular_buffer[FRQ_AVERAGING];     //!< Ring buffer for RPM averaging for tachometer (буфер усреднения частоты вращения коленвала для тахометра)
uint16_t temp_circular_buffer[TMP_AVERAGING];     //!< Ring buffer for averaging of coolant temperature (буфер усреднения температуры охлаждающей жидкости)
uint16_t int_temp_circular_buffer[TMP_AVERAGING];     //!< Ring buffer for averaging of IC temperature (буфер усреднения температуры микропроцессора)

void meas_init_ports(void)
{
 WRITEBIT (PORTB,PB1,1); // enable pullup
}

//обновление буферов усреднения (частота вращения, датчики...)
void meas_update_values_buffers(struct ecudata_t* d, uint8_t rpm_only)
{
 static uint8_t  tmp_ai  = TMP_AVERAGING-1;
 static uint8_t  frq_ai  = FRQ_AVERAGING-1;
 static uint8_t  int_tmp_ai = TMP_AVERAGING-1;

 freq_circular_buffer[frq_ai] = d->sens.inst_frq;
 (frq_ai==0) ? (frq_ai = FRQ_AVERAGING - 1): frq_ai--;

 if (rpm_only)
  return;

 temp_circular_buffer[tmp_ai] = adc_get_temp_value();
 (tmp_ai==0) ? (tmp_ai = TMP_AVERAGING - 1): tmp_ai--;
 
 int_temp_circular_buffer[int_tmp_ai] = adc_get_int_temp_value();
 (int_tmp_ai==0) ? (int_tmp_ai = TMP_AVERAGING - 1): int_tmp_ai--;
}


//усреднение измеряемых величин используя текущие значения кольцевых буферов усреднения, компенсация
//погрешностей АЦП, перевод измеренных значений в физические величины.
void meas_average_measured_values(struct ecudata_t* d)
{
 uint8_t i;  uint32_t sum;
 static uint16_t temp_avr = 0;

  for (sum=0,i = 0; i < TMP_AVERAGING; i++) //усредняем температуру (ДТОЖ)
  { //filter noisy samples by comparing each sample with averaged value (threshold is 6.25%)
   uint16_t t = (temp_avr >> 4);
   if (temp_avr && (temp_circular_buffer[i] > (temp_avr + t)))
    sum+=(temp_avr + t);
   else if (temp_avr && (temp_circular_buffer[i] < ((int16_t)temp_avr - t)))
    sum+=(temp_avr - t);
   else
    sum+=temp_circular_buffer[i];
  }
  temp_avr = sum/TMP_AVERAGING;
  d->sens.temperat_raw = adc_compensate(_RESDIV(temp_avr, 5, 3),PARAM_TEMP_ADC_FACTOR,PARAM_TEMP_ADC_CORRECTION);
if (!THERMISTOR_CS)
  d->sens.temperat = temp_adc_to_c(d->sens.temperat_raw);
else {
  if (!PARAM_CTS_USE_MAP) //use linear sensor
   d->sens.temperat = temp_adc_to_c(d->sens.temperat_raw);
  else //use lookup table (actual for thermistor sensors)
   d->sens.temperat = thermistor_lookup(d->sens.temperat_raw);
}

 for (sum=0, i=0; i < TMP_AVERAGING; i++) // усредняем температуру внутреннего датчика
    sum+=int_temp_circular_buffer[i];
 d->sens.int_temperat_raw = sum/TMP_AVERAGING;
 d->sens.int_temperat = int_temp_adc_to_c(d->sens.int_temperat_raw);
 restrict_value_to(&d->sens.int_temperat,TEMPERATURE_MAGNITUDE(-40),TEMPERATURE_MAGNITUDE(120));
 
 for (sum=0,i = 0; i < FRQ_AVERAGING; i++)  //усредняем частоту вращения коленвала
  sum+=freq_circular_buffer[i];
 d->sens.frequen=(sum/FRQ_AVERAGING);
 
 d->sens.frequen=((uint32_t)d->sens.frequen * simple_interpolation(d->sens.int_temperat/TEMP_PHYSICAL_MAGNITUDE_MULTIPLIER,
    PARAM_RC_CORRECTION_BEGIN_FREQ,PARAM_RC_CORRECTION_END_FREQ,PARAM_RC_CORRECTION_BEGIN_TEMP,
    PARAM_RC_CORRECTION_END_TEMP-PARAM_RC_CORRECTION_BEGIN_TEMP,1) / PARAM_RC_CALIBRATION_FREQ);
}

//Вызывать для предварительного измерения перед пуском двигателя. Вызывать только после
//инициализации АЦП.
void meas_initial_measure(struct ecudata_t* d)
{
 uint8_t _t,i = 16;
 _t = _SAVE_INTERRUPT();
 _ENABLE_INTERRUPT();
 do
 {
  adc_begin_measure(0); //<--normal speed
  while(!adc_is_measure_ready());

  meas_update_values_buffers(d, 0); //<-- all
 }while(--i);
 _RESTORE_INTERRUPT(_t);
 meas_average_measured_values(d);
}

void meas_take_discrete_inputs(struct ecudata_t *d)
{
 //--инверсия концевика карбюратора если необходимо
 d->sens.carb=PARAM_CARB_INVERS^GET_THROTTLE_GATE_STATE(); //результат: 0 - дроссель закрыт, 1 - открыт
}
