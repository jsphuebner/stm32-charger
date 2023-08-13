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

#define CDMSTAT  "0=Off, 1=Charging, 4=ConnLock"
#define OPMODES  "0=Off, 1=Precharge, 2=Run, 3=Exit"
#define PWMFRQS  "0=17.6kHz, 1=8.8kHz, 2=4.4KHz, 3=2.2kHz, 4=1.1kHz"
#define PWMPOLS  "0=ActHigh, 1=ActLow"
#define ONOFF    "0=Off, 1=On"
#define SNS_HS   "0=JCurve, 1=Semikron, 2=Prius"
#define DISPFUNCS "0=Current, 1=Power, 2=SoC, 3=TmpHs"
#define VER 4.23

#define MOD_OFF         0
#define MOD_PRECHARGE   1
#define MOD_RUN         2
#define MOD_EXIT        3

#define CDM_OFF         0
#define CDM_CHARGING    1
#define CDM_LOCK        4

#define DISP_CURRENT    0
#define DISP_POWER      1
#define DISP_SOC        2
#define DISP_TMPHS      3

#define CAT_POWERSTAGE  "Power Stage"
#define CAT_CONTROLLERS "Controllers"
#define CAT_CHARGER     "Charger Parameters"
#define CAT_AUX         "Auxilliary outputs"
#define CAT_RELAYS      "Contactor Control"

/* Entries must be ordered as follows:
   1. Saveable parameters (id != 0)
   2. Temporary parameters (id = 0)
   3. Display values
 */
//Next param id (increase when adding new parameter!): 78
/*               category  name         unit       min     max     default id */
#define PARAM_LIST \
    PARAM_ENTRY(CAT_POWERSTAGE,  pwmfrq,      PWMFRQS,   0,      4,      2,      13  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  pwmpol,      PWMPOLS,   0,      1,      0,      52  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  udclim,      "V",       0,      1000,   540,    48  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  ocurlim,     "A",       0,      1000,   -100,   22  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  tmphsmax,    "°C",      20,     100,    70,     1   ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  minpulse,    "dig",     0,      4095,   32,     24  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  idcgain,     "dig/A",   -100,   100,    10,     27  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  udcgain,     "dig/V",   0,      4095,   6.175,  29  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  udcofs,      "dig",     0,      4095,   0,      73  ) \
    PARAM_ENTRY(CAT_POWERSTAGE,  snshs,       SNS_HS,    0,      2,      0,      45  ) \
    PARAM_ENTRY(CAT_CONTROLLERS, pwmmin,      "dig",     0,      4095,   0,      75  ) \
    PARAM_ENTRY(CAT_CONTROLLERS, pwmmax,      "dig",     0,      4095,   3500,    2  ) \
    PARAM_ENTRY(CAT_CONTROLLERS, idckp,       "dig",     0,      1000,   20,     65  ) \
    PARAM_ENTRY(CAT_CONTROLLERS, idcki,       "dig",     0,      1000,   10,     66  ) \
    PARAM_ENTRY(CAT_CONTROLLERS, udcki,       "dig",     0,      1000,   10,     76  ) \
    PARAM_ENTRY(CAT_CONTROLLERS, idcflt,      "dig",     0,      16,     2,      67  ) \
    PARAM_ENTRY(CAT_CONTROLLERS, idcramp,     "A/s",     0,      50,     1,      69  ) \
    PARAM_ENTRY(CAT_CHARGER,     maxvtg,      "V",       8,      1000,   60,     15  ) \
    PARAM_ENTRY(CAT_CHARGER,     idclim,      "A",       0,      50,     25,     70  ) \
    PARAM_ENTRY(CAT_CHARGER,     idcspnt,     "A",       0,      50,     0,      71  ) \
    PARAM_ENTRY(CAT_CHARGER,     run,         ONOFF,     0,      1,      0,      0   ) \
    PARAM_ENTRY(CAT_AUX,         pwmgain,     "dig/C",   -65535, 65535,  100,    40  ) \
    PARAM_ENTRY(CAT_AUX,         pwmofs,      "dig",     -65535, 65535,  0,      41  ) \
    PARAM_ENTRY(CAT_AUX,         dispgain,    "dig/A",   -65535, 65535,  100,    68  ) \
    PARAM_ENTRY(CAT_AUX,         dispfunc,    DISPFUNCS, 0,      3,      0,      77  ) \
    PARAM_ENTRY(CAT_RELAYS,      udcsw,       "V",       0,      1000,   330,    20  ) \
    PARAM_ENTRY(CAT_RELAYS,      udcbatmin,   "V",       0,      1000,   330,    9   ) \
    PARAM_ENTRY(CAT_RELAYS,      precfrombat, ONOFF,     0,      1,      0,      74  ) \
    PARAM_ENTRY("Default",       version,     "4=1.11.R",4,      4,      4,      0   ) \
    VALUE_ENTRY(opmode,      OPMODES, 1000 ) \
    VALUE_ENTRY(cdmstat,     OPMODES, 1014 ) \
    VALUE_ENTRY(lasterr,     errorListString,  1013 ) \
    VALUE_ENTRY(udc,         "V",     1001 ) \
    VALUE_ENTRY(uout,        "V",     1014 ) \
    VALUE_ENTRY(idc,         "A",     1003 ) \
    VALUE_ENTRY(power,       "W",     1013 ) \
    VALUE_ENTRY(amp,         "dig",   1004 ) \
    VALUE_ENTRY(tmphs,       "°C",    1005 ) \
    VALUE_ENTRY(uaux,        "V",     1006 ) \
    VALUE_ENTRY(soc,         "%",     1002 ) \
    VALUE_ENTRY(din_start,   ONOFF,   1008 ) \
    VALUE_ENTRY(din_emcystop,ONOFF,   1009 ) \
    VALUE_ENTRY(din_ocur,    ONOFF,   1010 ) \
    VALUE_ENTRY(din_bms,     ONOFF,   1011 ) \
    VALUE_ENTRY(tm_meas,     "us",    1012 ) \

//Generated enum-string for possible errors
extern const char* errorListString;
