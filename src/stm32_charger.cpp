/*
 * This file is part of the tumanako_vc project.
 *
 * Copyright (C) 2010 Johannes Huebner <contact@johanneshuebner.com>
 * Copyright (C) 2010 Edward Cheeseman <cheesemanedward@gmail.com>
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * History:
 * Example from libopencm3 expanded to make tumanako arm tester
 * Added sine wave generation, removed testing code
 */
#include <stdint.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/rtc.h>
#include "stm32scheduler.h"
#include "stm32_can.h"
#include "terminal.h"
#include "params.h"
#include "hwdefs.h"
#include "digio.h"
#include "hwinit.h"
#include "anain.h"
#include "temp_meas.h"
#include "param_save.h"
#include "my_math.h"

#define CAN_TIMEOUT       50  //500ms

static uint8_t  pwmdigits;
static uint16_t pwmfrq;
static uint16_t pwmmax;
static s32fp idcspnt = 0;
//Current sensor offsets are determined at runtime
static int idcofs;
static Stm32Scheduler* scheduler;

static void SetCurrentLimitThreshold();

extern "C" void tim1_brk_isr(void)
{
   timer_disable_irq(PWM_TIMER, TIM_DIER_BIE);
   Param::SetInt(Param::opmode, MOD_OFF);
   Param::SetInt(Param::idc, 0);
}

extern "C" void pwm_timer_isr(void)
{
   static s32fp esum = 0;
   static int idcflt = 0;
   uint16_t last = timer_get_counter(PWM_TIMER);
   /* Clear interrupt pending flag */
   TIM_SR(PWM_TIMER) &= ~TIM_SR_UIF;

   idcflt = IIRFILTER(idcflt, AnaIn::idc.Get(), Param::GetInt(Param::idcflt));
   if (MOD_RUN != Param::GetInt(Param::opmode))
   {
      idcofs = idcflt;
      esum = 0;
      SetCurrentLimitThreshold();
      timer_set_oc_value(PWM_TIMER, TIM_OC1, 0);
      timer_set_oc_value(PWM_TIMER, TIM_OC2, 0);
   }
   else
   {
      s32fp kp = Param::Get(Param::idckp);
      s32fp ki = Param::Get(Param::idcki);
      s32fp idc = FP_DIV(FP_FROMINT(idcflt - idcofs), Param::Get(Param::idcgain));
      s32fp idcerr = idcspnt - idc;
      s32fp out = FP_MUL(idcerr, kp) + (esum * ki / pwmfrq);
      uint16_t pwm = MAX(0, MIN(pwmmax, FP_TOINT(out)));
      uint16_t disp = MAX(0, FP_TOINT(FP_MUL(Param::Get(Param::dispgain), idc)));

      if (idcspnt == 0)
         esum = 0;

      if ((pwm > 0 && pwm < pwmmax && esum < 5000000) || idcerr < 0)
         esum += idcerr;

      if (pwm < Param::GetInt(Param::minpulse))
         pwm = 0;

      if (pwm > (pwmmax - Param::GetInt(Param::minpulse)))
         pwm = pwmmax - Param::GetInt(Param::minpulse);

      timer_set_oc_value(PWM_TIMER, TIM_OC1, pwm);
      timer_set_oc_value(PWM_TIMER, TIM_OC2, disp);
      Param::SetFlt(Param::idc, idc);
      Param::SetInt(Param::amp, pwm);
   }

   Param::SetInt(Param::tm_meas, (timer_get_counter(PWM_TIMER) - last)/72);
}

static void PwmInit(void)
{
   pwmdigits = MIN_PWM_DIGITS + Param::GetInt(Param::pwmfrq);
   pwmfrq = tim_setup(pwmdigits, Param::GetInt(Param::pwmpol));
   pwmmax = (1 << pwmdigits) - 1;
   pwmmax = MIN(pwmmax, Param::GetInt(Param::pwmmax));
}


static void Ms100Task(void)
{
   static bool derate = false;
   s32fp _idcspnt = MIN(Param::Get(Param::idcspnt), Param::Get(Param::idclim));
   s32fp idcIncr = Param::Get(Param::idcramp) / 10;
   s32fp tmphs = Param::Get(Param::tmphs);
   s32fp tmphsmax = Param::Get(Param::tmphsmax);
   uint32_t lastCanReceptionTime = Can::GetInterface(0)->GetLastRxTimestamp();

   /* If we ever received something via CAN, assume CAN controlled mode */
   if (lastCanReceptionTime > 0 && (rtc_get_counter_val() - lastCanReceptionTime) >= CAN_TIMEOUT)
   {
      _idcspnt = 0;
   }

   if (tmphs < (tmphsmax - FP_FROMINT(5)))
   {
      derate = false;
   }
   else if (tmphs > tmphsmax || derate)
   {
      _idcspnt = 0;
      derate = true;
   }

   if ((idcspnt + idcIncr) <= _idcspnt)
      idcspnt += idcIncr;
   else if (idcspnt < _idcspnt || idcspnt > _idcspnt)
      idcspnt = _idcspnt;
   else
      idcspnt = _idcspnt;


   DigIo::led_out.Toggle();
   iwdg_reset();

   Param::SetInt(Param::din_start, DigIo::start_in.Get());
   Param::SetInt(Param::din_emcystop, DigIo::emcystop_in.Get());
   Param::SetInt(Param::din_bms, DigIo::bms_in.Get());

   Can::GetInterface(0)->SendAll();
}


static void CalcAndOutputTemp()
{
   static int temphs = 0;
   int pwmgain = Param::GetInt(Param::pwmgain);
   int pwmofs = Param::GetInt(Param::pwmofs);
   int tmpout;
   TempMeas::Sensors snshs = (TempMeas::Sensors)Param::GetInt(Param::snshs);
   temphs = IIRFILTER(AnaIn::tmphs.Get(), temphs, 15);
   s32fp tmphsf = TempMeas::Lookup(temphs, snshs);
   tmpout = FP_TOINT(tmphsf) * pwmgain + pwmofs;
   tmpout = MIN(0xFFFF, MAX(0, tmpout));

   if (tmpout < 100)
   {
      if (tmpout > 50)
         tmpout = 100;
      else
         tmpout = 0;
   }

   timer_set_oc_value(OVER_CUR_TIMER, TIM_OC4, tmpout);

   Param::SetFlt(Param::tmphs, tmphsf);
}

static s32fp ProcessUdc()
{
   static int32_t udc = 0;
   s32fp udcfp;
   s32fp udclim = Param::Get(Param::udclim);
   s32fp udcgain = Param::Get(Param::udcgain);

   Param::SetFlt(Param::uaux, FP_DIV(AnaIn::uaux.Get(), 250));
   udc = IIRFILTER(udc, AnaIn::udc.Get(), 5);
   udcfp = FP_DIV(FP_FROMINT(udc), udcgain);

   if (udcfp > udclim)
   {
      Param::SetInt(Param::opmode, MOD_OFF);
   }

   Param::SetFlt(Param::udc, udcfp);

   return udcfp;
}

static void Ms10Task(void)
{
   s32fp udc = ProcessUdc();
   int opmode = Param::GetInt(Param::opmode);
   static int initWait = 0;

   CalcAndOutputTemp();

   /* switch on DC switch above threshold */
   if (opmode == MOD_OFF && udc >= Param::Get(Param::udcsw) && (DigIo::start_in.Get() || Param::GetInt(Param::run) == 1))
   {
      opmode = MOD_WAIT;
      DigIo::dcsw_out.Set();
      Param::SetInt(Param::run, 0); //reset so it doesn't restart on error
      Param::SetInt(Param::opmode, MOD_WAIT);
   }

   if (initWait < 0 && DigIo::start_in.Get())
   {
      opmode = MOD_EXIT;
      Param::SetInt(Param::opmode, MOD_EXIT);
      initWait = 100;
      idcspnt = 0;
   }

   if (MOD_OFF == opmode)
   {
      initWait = 500;
      idcspnt = 0;
      tim_output_disable();
      DigIo::dcsw_out.Clear();
      DigIo::outc_out.Clear();
   }
   else if (MOD_EXIT == opmode)
   {
      if (initWait > 0)
      {
         initWait--;
      }
      else
      {
         Param::SetInt(Param::opmode, MOD_OFF);
      }
   }
   else if (100 == initWait)
   {
      DigIo::outc_out.Set();
      initWait--;
   }
   else if (0 == initWait)
   {
      PwmInit();
      tim_output_enable();
      initWait = -1;
      Param::SetInt(Param::opmode, MOD_RUN);
   }
   else if (initWait > 0)
   {
      initWait--;
   }
}

static void SetCurrentLimitThreshold()
{
   s32fp ocurlim = Param::Get(Param::ocurlim);
   s32fp igain = Param::Get(Param::idcgain);

   if (igain < 0) igain = -igain;

   ocurlim = FP_MUL(igain, ocurlim);
   int limNeg = idcofs - FP_TOINT(ocurlim);
   int limPos = idcofs + FP_TOINT(ocurlim);
   limNeg = MAX(0, limNeg);
   limPos = MIN(OCURMAX, limPos);

   timer_set_oc_value(OVER_CUR_TIMER, TIM_OC3, limNeg);
   timer_set_oc_value(OVER_CUR_TIMER, TIM_OC2, limPos);
}

/** This function is called when the user changes a parameter */
extern void parm_Change(Param::PARAM_NUM ParamNum)
{
   if (ParamNum == Param::idcspnt)
   {
      Param::SetFlt(Param::idcspnt, MIN(Param::Get(Param::idcspnt), Param::Get(Param::idclim)));
   }
   SetCurrentLimitThreshold();
}

extern "C" void tim2_isr(void)
{
   scheduler->Run();
}

extern "C" int main(void)
{
   clock_setup();

   ANA_IN_CONFIGURE(ANA_IN_LIST);
   DIG_IO_CONFIGURE(DIG_IO_LIST);

   usart_setup();
   nvic_setup();
   rtc_setup();
   AnaIn::Start();
   parm_load();
   parm_Change(Param::PARAM_LAST);
   PwmInit();

   Stm32Scheduler s(TIM2); //We never exit main so it's ok to put it on stack
   scheduler = &s;
   Can c(CAN1, Can::Baud500);

   s.AddTask(Ms100Task, 100);
   s.AddTask(Ms10Task, 10);

   term_Run();

   return 0;
}

