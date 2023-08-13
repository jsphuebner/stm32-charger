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
#include "errormessage.h"

#define CAN_TIMEOUT       50  //500ms

static uint8_t  pwmdigits;
static uint16_t pwmfrq;
static uint16_t pwmmax;
static s32fp idcspnt = 0;
static s32fp esum = 0;
//Current sensor offsets are determined at runtime
static int idcofs;
static Stm32Scheduler* scheduler;

static void SetCurrentLimitThreshold();

extern "C" void tim1_brk_isr(void)
{
   timer_disable_irq(PWM_TIMER, TIM_DIER_BIE);
   Param::SetInt(Param::opmode, MOD_OFF);
   //Param::SetInt(Param::idc, 0);
   Param::SetInt(Param::run, false);
   DigIo::pwmdis_out.Set();

   ErrorMessage::Post(ERR_OVERCURRENT);
}

extern "C" void pwm_timer_isr(void)
{
   static int idcflt = 0;
   uint16_t last = timer_get_counter(PWM_TIMER);
   int opmode = Param::GetInt(Param::opmode);
   int idcAdc = AnaIn::idc.Get();
   /* Clear interrupt pending flag */
   TIM_SR(PWM_TIMER) &= ~TIM_SR_UIF;

   idcflt = IIRFILTER(idcflt, idcAdc, Param::GetInt(Param::idcflt));
   s32fp idc = FP_DIV(FP_FROMINT(idcflt - idcofs), Param::Get(Param::idcgain));

   if (MOD_PRECHARGE == opmode)
   {
      s32fp udcerr = Param::GetInt(Param::udcsw) - Param::GetInt(Param::udc);
      int out = pwmmax - ((Param::GetInt(Param::udcki) * esum) / pwmfrq);

      if (out > pwmmax / 3)
         esum += udcerr;

      out = MAX(pwmmax / 3, out);
      out = MIN(pwmmax, out);
      uint16_t disp = MAX(0, FP_TOINT(FP_MUL(Param::Get(Param::dispgain), Param::Get(Param::udc) / 10)));

      timer_set_oc_value(PWM_TIMER, TIM_OC1, out);
      timer_set_oc_value(PWM_TIMER, TIM_OC2, disp);
      DigIo::pwmdis_out.Clear();
      Param::SetInt(Param::amp, out);
   }
   else if (MOD_RUN == opmode || MOD_EXIT == opmode)
   {
      s32fp kp = Param::Get(Param::idckp);
      s32fp ki = Param::Get(Param::idcki);
      s32fp idcerr = idcspnt - idc;
      s32fp out = FP_MUL(idcerr, kp) + (esum * ki / pwmfrq);
      uint16_t pwm = MAX(0, MIN(pwmmax, FP_TOINT(out)));

      pwm = MAX(Param::GetInt(Param::pwmmin), pwm);

      if ((pwm > 0 && pwm < pwmmax && esum < 5000000) || idcerr < 0)
         esum += idcerr;

      if (pwm < Param::GetInt(Param::minpulse))
         pwm = 0;

      if (pwm > (pwmmax - Param::GetInt(Param::minpulse)))
         pwm = pwmmax - Param::GetInt(Param::minpulse);

      if (idcspnt == 0)
      {
         esum = 0;
         pwm = Param::GetInt(Param::pwmmin);
         DigIo::pwmdis_out.Set();
      }

      int uout = (Param::GetInt(Param::udc) * pwm) / pwmmax;
      s32fp power = uout * idc;

      timer_set_oc_value(PWM_TIMER, TIM_OC1, pwm);

      if (idcspnt > 0)
      {
         DigIo::pwmdis_out.Clear();
      }

      Param::SetInt(Param::amp, pwm);
      Param::SetInt(Param::uout, uout);
      Param::SetFixed(Param::power, power);
   }

   Param::SetFixed(Param::idc, idc);
   Param::SetInt(Param::tm_meas, (timer_get_counter(PWM_TIMER) - last)/72);
}

static void PwmInit(void)
{
   idcofs = AnaIn::idc.Get();
   SetCurrentLimitThreshold();

   pwmdigits = MIN_PWM_DIGITS + Param::GetInt(Param::pwmfrq);
   pwmfrq = tim_setup(pwmdigits, Param::GetInt(Param::pwmpol));
   pwmmax = (1 << pwmdigits) - 1;
   pwmmax = MIN(pwmmax, Param::GetInt(Param::pwmmax));
   esum = 0;
}

static void CalcAndOutputTemp()
{
   static int temphs = 0;
   int pwmgain = Param::GetInt(Param::pwmgain);
   int pwmofs = Param::GetInt(Param::pwmofs);
   int tmpout;
   TempMeas::Sensors snshs = (TempMeas::Sensors)Param::GetInt(Param::snshs);
   float tmphsf;
   temphs = IIRFILTER(AnaIn::tmphs.Get(), temphs, 8);

   if (snshs == TempMeas::TEMP_PRIUS)
   {
      tmphsf = 166.66f - temphs / 18.62f;
   }
   else
   {
      tmphsf = TempMeas::Lookup(temphs, snshs);
   }

   tmpout = tmphsf * pwmgain + pwmofs;
   tmpout = MIN(0xFFFF, MAX(0, tmpout));

   if (tmpout < 100)
   {
      if (tmpout > 50)
         tmpout = 100;
      else
         tmpout = 0;
   }

   timer_set_oc_value(OVER_CUR_TIMER, TIM_OC4, tmpout);

   Param::SetFloat(Param::tmphs, tmphsf);
}

static void UpdateDisplay()
{
   int opmode = Param::GetInt(Param::opmode);
   int dispfunc = Param::GetInt(Param::dispfunc);
   s32fp value = Param::Get(Param::soc);

   if (MOD_PRECHARGE == opmode)
   {
      value = Param::Get(Param::udc);
   }
   else if (MOD_RUN == opmode)
   {
      switch (dispfunc)
      {
      case DISP_CURRENT:
         value = Param::Get(Param::idc);
         break;
      case DISP_POWER:
         value = Param::Get(Param::power) / 1000;
         break;
      case DISP_SOC:
         value = Param::Get(Param::soc) / 10;
         break;
      case DISP_TMPHS:
         value = Param::Get(Param::tmphs);
      }
   }

   uint16_t disp = MAX(0, FP_TOINT(FP_MUL(Param::Get(Param::dispgain), value)));
   timer_set_oc_value(PWM_TIMER, TIM_OC2, disp);
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
   if ((lastCanReceptionTime > 0 && (rtc_get_counter_val() - lastCanReceptionTime) >= CAN_TIMEOUT))
   {
      _idcspnt = 0;
      ErrorMessage::Post(ERR_CANTIMEOUT);
   }
   else if (Param::GetInt(Param::opmode) != MOD_RUN)
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

   CalcAndOutputTemp();
   UpdateDisplay();

   DigIo::led_out.Toggle();
   iwdg_reset();

   ErrorMessage::SetTime(rtc_get_counter_val());

   Param::SetInt(Param::din_start, DigIo::start_in.Get());
   Param::SetInt(Param::din_emcystop, DigIo::emcystop_in.Get());
   Param::SetInt(Param::din_bms, DigIo::bms_in.Get());
   Param::SetInt(Param::lasterr, ErrorMessage::GetLastError());

   Can::GetInterface(0)->SendAll();
}

static float ProcessUdc()
{
   static int32_t udc = 0;
   float udcfp;
   float udclim = Param::GetFloat(Param::udclim);
   float udcgain = Param::GetFloat(Param::udcgain);

   Param::SetFloat(Param::uaux, AnaIn::uaux.Get() / 250.0f);
   udc = IIRFILTER(udc, AnaIn::udc.Get(), 4);
   udcfp = (udc - Param::GetInt(Param::udcofs)) / udcgain;

   if (udcfp > udclim)
   {
      Param::SetInt(Param::opmode, MOD_OFF);
      ErrorMessage::Post(ERR_OVERVOLTAGE);
   }

   Param::SetFloat(Param::udc, udcfp);

   return udcfp;
}

static void Ms10Task(void)
{
   float udc = ProcessUdc();
   int opmode = Param::GetInt(Param::opmode);
   static int initWait = 0;

   bool run = Param::GetBool(Param::run);

   /* switch on DC switch above threshold */
   if (opmode == MOD_OFF &&
      (udc >= Param::GetFloat(Param::udcsw) || Param::GetBool(Param::precfrombat)) &&
      !DigIo::bms_in.Get() && (DigIo::start_in.Get() || run))
   {
      if (Param::GetBool(Param::precfrombat))
      {
         PwmInit();
         timer_set_oc_value(PWM_TIMER, TIM_OC1, pwmmax);
         tim_output_enable();
         DigIo::pwmdis_out.Clear();
      }

      run = true;
      opmode = MOD_PRECHARGE;
      DigIo::dcsw_out.Set();
      Param::SetInt(Param::opmode, MOD_PRECHARGE);
      Param::SetInt(Param::run, true);

      ErrorMessage::UnpostAll();
   }

   if ((initWait < 0 && DigIo::start_in.Get()) ||
       (opmode != MOD_OFF && opmode != MOD_EXIT && (DigIo::bms_in.Get() || !run)))
   {
      opmode = MOD_EXIT;
      Param::SetInt(Param::opmode, MOD_EXIT);
      initWait = 100;
      idcspnt = 0;
   }

   if (MOD_OFF == opmode)
   {
      initWait = 50;
      idcspnt = 0;
      tim_output_disable();
      DigIo::pwmdis_out.Set();
      DigIo::dcsw_out.Clear();
      DigIo::outc_out.Clear();
      DigIo::acsw_out.Clear();
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
   else if (opmode == MOD_PRECHARGE)
   {
      if (udc >= Param::GetFloat(Param::udcbatmin))
      {
         Param::SetInt(Param::pwmmin, Param::GetInt(Param::amp));
         //DigIo::pwmdis_out.Set();
         //tim_output_disable();
         DigIo::outc_out.Set();
         DigIo::acsw_out.Set();
         initWait = 100;
         Param::SetInt(Param::opmode, MOD_RUN);
      }
   }
   else if (0 == initWait)
   {
      PwmInit();
      tim_output_enable();
      initWait = -1;
   }
   else if (initWait > 0)
   {
      initWait--;
   }
}

static void SetCurrentLimitThreshold()
{
   s32fp ocurlim = Param::Get(Param::ocurlim);
   s32fp igain = ABS(Param::Get(Param::idcgain));

   ocurlim = FP_MUL(igain, ocurlim);
   int limNeg = idcofs - FP_TOINT(ocurlim);
   int limPos = idcofs + FP_TOINT(ocurlim);
   limNeg = MAX(0, limNeg);
   limPos = MIN(OCURMAX, limPos);

   if (ocurlim != 0)
   {
      timer_enable_break(PWM_TIMER);
   }

   timer_set_oc_value(OVER_CUR_TIMER, TIM_OC2, limNeg);
   timer_set_oc_value(OVER_CUR_TIMER, TIM_OC3, limPos);
}

/** This function is called when the user changes a parameter */
void Param::Change(Param::PARAM_NUM ParamNum)
{
   if (ParamNum == Param::idcspnt)
   {
      Param::SetFixed(Param::idcspnt, MIN(Param::Get(Param::idcspnt), Param::Get(Param::idclim)));
   }
}

extern "C" void tim2_isr(void)
{
   scheduler->Run();
}

extern "C" int main(void)
{
   extern const TERM_CMD TermCmds[];

   clock_setup();

   ANA_IN_CONFIGURE(ANA_IN_LIST);
   DIG_IO_CONFIGURE(DIG_IO_LIST);

   nvic_setup();
   rtc_setup();
   AnaIn::Start();
   parm_load();
   Param::Change(Param::PARAM_LAST);
   PwmInit();

   Stm32Scheduler s(TIM2); //We never exit main so it's ok to put it on stack
   scheduler = &s;
   Can c(CAN1, Can::Baud500);
   Terminal t(USART3, TermCmds);

   s.AddTask(Ms100Task, 100);
   s.AddTask(Ms10Task, 10);

   while(1)
      t.Run();

   return 0;
}

