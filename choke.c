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

/** \file choke.c
 * Implementation of carburetor choke control.
 * (Реализация управления воздушной заслонкой карбюратора).
 */

#include <stdlib.h>
#include "bitmask.h"
#include "ecudata.h"
#include "funconv.h"
#include "magnitude.h"
#include "smcontrol.h"
#include "compilopt.h"

/**Direction used to set choke to the initial position */
#define INIT_POS_DIR SM_DIR_CW

#define USE_THROTTLE_POS 1         //undefine this constant if you don't need to use throttle limit switch in choke initialization
#define USE_RPMREG_TURNON_DELAY 1  //undefine this constant if you don't need delay

/**RPM regulator call period, 50ms*/
#define RPMREG_CORR_TIME 10

/**During this time system can't exit from RPM regulation mode*/
#define RPMREG_ENEX_TIME (10*100)

#ifdef USE_RPMREG_TURNON_DELAY
#define RPMREG_ENTO_TIME (3*100)
#endif

//See flags variable in choke_st_t
#define CF_RPMREG_ENEX  2  //!< flag which indicates that it is allowed to exit from RPM regulation mode
#define CF_SMDIR_CHG    3  //!< flag, indicates that stepper motor direction has changed during motion
#ifdef USE_RPMREG_TURNON_DELAY
#define CF_PRMREG_ENTO  4  //!< indicates that system is entered to RPM regulation mode
#endif
#define CF_CUR_DIR      5

/**Define state variables*/
typedef struct
{
 uint8_t   state;          //!< state machine state
 uint16_t  smpos;          //!< current position of stepper motor in steps
 int16_t   prev_temp;      //!< used for choke_closing_lookup()
 int16_t   smpos_prev;     //!< start value of stepper motor position (before each motion)
 uint8_t   strt_mode;      //!< state machine state used for starting mode
 uint16_t  strt_t1;        //!< used for time calculations by calc_startup_corr()
 uint8_t   flags;          //!< state flags (see CF_ definitions)
 int16_t   rpmreg_prev;    //!< previous value of RPM regulator
 uint16_t  rpmreg_t1;      //!< used to call RPM regulator function
 uint16_t  rpmval_prev;    //!< used to store RPM value to detect exit from RPM regulation mode
}choke_st_t;

/**Instance of state variables */
choke_st_t chks = {0};

void choke_init_ports(void)
{
 stpmot_init_ports();
}

void choke_init(void)
{
 chks.state = 0;
 chks.strt_mode = 0;
 chks.rpmreg_prev = 0;
 CLEARBIT(chks.flags, CF_RPMREG_ENEX);
}

/** Calculates choke position (%*2) from step value
 * \param value Position of choke in stepper motor steps
 * \param steps total number of steps
 * \return choke position in %*2
 */
uint8_t calc_percent_pos(uint16_t value, uint16_t steps)
{
 return (((uint32_t)value) * 200) / steps;
}

/** Calculates choke position correction at startup mode and from RPM regulator
 * Work flow: Start-->Wait 3 sec.-->RPM regul.-->Ready
 * \param d pointer to ECU data structure
 */
int16_t calc_startup_corr(struct ecudata_t* d)
{
 int16_t rpm_corr = 0;
 switch(chks.strt_mode)
 {
  case 0:  //starting
   if (d->st_block)
   {
    chks.strt_t1 = s_timer_gtc();
    chks.strt_mode = 1;
    //set choke RPM regulation flag (will be activated after delay)
    d->choke_rpm_reg = (0!=PARAM_CHOKE_RPM_0);
   }
   break; //use startup correction
  case 1:
   if ((s_timer_gtc() - chks.strt_t1) >= PARAM_CHOKE_CORR_TIME)
   {
    chks.strt_mode = 2;
    chks.rpmreg_prev = 0; //we will enter RPM regulation mode with zero correction
    chks.rpmval_prev = d->sens.frequen;
    chks.strt_t1 = s_timer_gtc();     //set timer to prevent RPM regulation exiting during set period of time
    chks.rpmreg_t1 = s_timer_gtc();
    chokerpm_regulator_init();
    CLEARBIT(chks.flags, CF_RPMREG_ENEX);
#ifdef USE_RPMREG_TURNON_DELAY
    CLEARBIT(chks.flags, CF_PRMREG_ENTO);
#endif
   }
   break; //use startup correction
  case 2:
  {
   uint16_t tmr = s_timer_gtc();
   if ((tmr - chks.rpmreg_t1) >= RPMREG_CORR_TIME)
   {
    chks.rpmreg_t1 = tmr;  //reset timer
    if ((tmr - chks.strt_t1) >= RPMREG_ENEX_TIME) //do we ready to enable RPM regulation mode exiting?
     SETBIT(chks.flags, CF_RPMREG_ENEX);
#ifdef USE_RPMREG_TURNON_DELAY
    if ((tmr - chks.strt_t1) >=  RPMREG_ENTO_TIME)
     SETBIT(chks.flags, CF_PRMREG_ENTO);
    if (CHECKBIT(chks.flags, CF_PRMREG_ENTO))
#endif
    rpm_corr = choke_rpm_regulator(d, &chks.rpmreg_prev);
    //detect fast throttle opening only if RPM > 1000
    if (d->sens.temperat >= (PARAM_IDLREG_TURN_ON_TEMP /*+ 1*/) ||
       (CHECKBIT(chks.flags, CF_RPMREG_ENEX) && (d->sens.frequen > 1000) && (((int16_t)d->sens.frequen - (int16_t)chks.rpmval_prev) > 180)))
    {
     chks.strt_mode = 3; //exit
     rpm_corr = 0;
     d->choke_rpm_reg = 0;
    }
    else
     chks.rpmval_prev = d->sens.frequen;
   }
   else
    rpm_corr = chks.rpmreg_prev;
  }
  case 3:
   if (!d->st_block)
    chks.strt_mode = 0; //engine is stopped, so use correction again
   return rpm_corr;  //correction from RPM regulator only
 }

 if (d->sens.temperat > PARAM_CHOKE_CORR_TEMP)
  return 0; //Do not use correction if coolant temperature > threshold
 else if (d->sens.temperat < TEMPERATURE_MAGNITUDE(0))
  return PARAM_CHOKE_SM_STEPS; //if temperature  < 0, then choke must be fully closed
 else
  return (((int32_t)PARAM_CHOKE_SM_STEPS) * PARAM_CHOKE_STARTUP_CORR) / 200;
}

/** Set choke to initial position. Because we have no position feedback, we
 * must use number of steps more than stepper actually has.
 * \param d pointer to ECU data structure
 * \param dir Direction (see description on stpmot_dir() function)
 */
static void initial_pos(struct ecudata_t* d, uint8_t dir)
{
 stpmot_dir(dir);                                             //set direction
#ifdef USE_THROTTLE_POS
 if (0==d->sens.carb)
  stpmot_run(PARAM_CHOKE_SM_STEPS/4);                         //run using number of steps = 25%
 else
#endif
  stpmot_run(PARAM_CHOKE_SM_STEPS + (PARAM_CHOKE_SM_STEPS/32));   //run using number of steps + 3%
}

/** Stepper motor control for normal working mode
 * \param d pointer to ECU data structure
 * \param pos current calculated (target) position of stepper motor
 */
void sm_motion_control(struct ecudata_t* d, int16_t pos)
{
 int16_t diff;
 uint8_t dir;
 restrict_value_to(&pos, 0, PARAM_CHOKE_SM_STEPS);
 if (CHECKBIT(chks.flags, CF_SMDIR_CHG))                      //direction has changed
 {
  if (!stpmot_is_busy())
  {
   chks.smpos = chks.smpos_prev + (!CHECKBIT(chks.flags,CF_CUR_DIR) ? -stpmot_stpcnt() : stpmot_stpcnt());
   CLEARBIT(chks.flags, CF_SMDIR_CHG);
  }
 }
 if (!CHECKBIT(chks.flags, CF_SMDIR_CHG))                     //normal operation
 {
  diff = pos - chks.smpos;
  if (!stpmot_is_busy())
  {
   if (diff != 0)
   {
    dir = diff < 0 ? 0:1;
    WRITEBIT (chks.flags,CF_CUR_DIR,dir);
    stpmot_dir(dir);
    stpmot_run(abs(diff));                                    //start stepper motor
    chks.smpos_prev = chks.smpos;                             //remember position when SM started motion
    chks.smpos = pos;                                         //this is a target position
   }
  }
  else //busy
  {
   //Check if curent target direction is not match new target direction. If it is not match, then
   //stop stepper motor and go to the direction changing.
   if (((chks.smpos - chks.smpos_prev) & 0x8000) != ((pos - chks.smpos_prev) & 0x8000))
   {
    stpmot_run(0);                                            //stop stepper motor
    SETBIT(chks.flags, CF_SMDIR_CHG);
   }
  }
 }
}

/** Calculate stepper motor position for normal mode
 * \param d pointer to ECU data structure
 * \param pwm 1 - PWM IAC, 0 - SM IAC
 * \return stepper motor position in steps
 */
int16_t calc_sm_position(struct ecudata_t* d, uint8_t pwm)
{
  return ((((int32_t)PARAM_CHOKE_SM_STEPS) * choke_closing_lookup(d, &chks.prev_temp)) / 200) + calc_startup_corr(d);
}

void choke_control(struct ecudata_t* d)
{
 int16_t pos;
 switch(chks.state)
 {
  case 0:                                                     //Initialization of choke position  
   initial_pos(d, INIT_POS_DIR);
   chks.state = 2;
   chks.prev_temp = d->sens.temperat;
   break;

  case 1:                                                     //system is being preparing to power-down
   initial_pos(d, INIT_POS_DIR);
   chks.state = 2;
   break;

  case 2:                                                     //wait while choke is being initialized
   if (!stpmot_is_busy())                                     //ready?
   {
    chks.state = 5;                                          //normal working
    chks.smpos = 0;                                           //initial position (fully opened)
    CLEARBIT(chks.flags, CF_SMDIR_CHG);
   }
   break;

  case 5:                                                     //normal working mode
   pos = calc_sm_position(d, 0);                            //calculate stepper motor position
   sm_motion_control(d, pos);                                //SM command execution
   d->choke_pos = calc_percent_pos(chks.smpos, PARAM_CHOKE_SM_STEPS);//update position value
   break;
   
  default:
   break;
 }
}
