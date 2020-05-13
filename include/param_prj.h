/*
 * This file is part of the tumanako_vc project.
 *
 * Copyright (C) 2011 Johannes Huebner <dev@johanneshuebner.com>
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

#define OPMODES "0=Off, 1=Wait, 2=Run"
#define PWMFRQS  "0=17.6kHz, 1=8.8kHz, 2=4.4KHz, 3=2.2kHz, 4=1.1kHz"
#define PWMPOLS  "0=ACTHIGH, 1=ACTLOW"
#define ONOFF    "0=OFF, 1=ON"
#define SNS_HS   "0=JCurve, 1=Semikron"
#define SNS_M    "2=KTY83-110, 3=KTY84-130"
#define PWMFUNCS "0=tmpm, 1=tmphs, 2=speed"
#define VER 4.00

#define MOD_OFF    0
#define MOD_WAIT   1
#define MOD_RUN    2

#define PWM_FUNC_TMPM  0
#define PWM_FUNC_TMPHS 1
#define PWM_FUNC_SPEED 2


/* Entries must be ordered as follows:
   1. Saveable parameters (id != 0)
   2. Temporary parameters (id = 0)
   3. Display values
 */
//Next param id (increase when adding new parameter!): 71
/*               category  name         unit       min     max     default id */
#define PARAM_LIST \
    PARAM_ENTRY("Default", pwmfrq,      PWMFRQS,   0,      4,      2,      13  ) \
    PARAM_ENTRY("Default", pwmpol,      PWMPOLS,   0,      1,      0,      52  ) \
    PARAM_ENTRY("Default", deadtime,    "dig",     0,      255,    28,     14  ) \
    PARAM_ENTRY("Default", maxvtg,      "V",       8,      1000,   60,     15  ) \
    PARAM_ENTRY("Default", udcsw,       "V",       0,      1000,   330,    20  ) \
    PARAM_ENTRY("Default", udclim,      "V",       0,      1000,   540,    48  ) \
    PARAM_ENTRY("Default", ocurlim,     "A",       -1000,  1000,   -100,   22  ) \
    PARAM_ENTRY("Default", tmphsmax,    "°C",      20,     100,    70,     1   ) \
    PARAM_ENTRY("Default", minpulse,    "dig",     0,      4095,   32,     24  ) \
    PARAM_ENTRY("Default", idcgain,     "dig/A",   -100,   100,    -4.7,   27  ) \
    PARAM_ENTRY("Default", udcgain,     "dig/V",   0,      4095,   6.175,  29  ) \
    PARAM_ENTRY("Default", pwmgain,     "dig/C",   -65535, 65535,  100,    40  ) \
    PARAM_ENTRY("Default", pwmofs,      "dig",     -65535, 65535,  0,      41  ) \
    PARAM_ENTRY("Default", dispgain,    "dig/A",   -65535, 65535,  100,    68  ) \
    PARAM_ENTRY("Default", snshs,       SNS_HS,    0,      1,      0,      45  ) \
    PARAM_ENTRY("Default", idckp,       "dig",     0,      1000,   1,      65  ) \
    PARAM_ENTRY("Default", idcki,       "dig",     0,      1000,   1,      66  ) \
    PARAM_ENTRY("Default", idcflt,      "dig",     0,      16,     1,      67  ) \
    PARAM_ENTRY("Default", idcramp,     "A/s",     0,      50,     1,      69  ) \
    PARAM_ENTRY("Default", idclim,      "A",       0,      50,     25,     70   ) \
    PARAM_ENTRY("Default", idcspnt,     "A",       0,      50,     0,      0   ) \
    PARAM_ENTRY("Default", run,         ONOFF,     0,      1,      0,      0   ) \
    PARAM_ENTRY("Default", version,     "",        0,      0,      VER,    0   ) \
    VALUE_ENTRY(opmode,      OPMODES, 1000 ) \
    VALUE_ENTRY(udc,         "V",     1001 ) \
    VALUE_ENTRY(idc,         "A",     1003 ) \
    VALUE_ENTRY(amp,         "dig",   1004 ) \
    VALUE_ENTRY(tmphs,       "°C",    1005 ) \
    VALUE_ENTRY(uaux,        "V",     1006 ) \
    VALUE_ENTRY(din_start,   ONOFF,   1008 ) \
    VALUE_ENTRY(din_emcystop,ONOFF,   1009 ) \
    VALUE_ENTRY(din_ocur,    ONOFF,   1010 ) \
    VALUE_ENTRY(din_bms,     ONOFF,   1011 ) \
    VALUE_ENTRY(tm_meas,     "us",    1012 ) \

