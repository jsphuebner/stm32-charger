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

#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/usart.h>
#include "hwdefs.h"
#include "terminal.h"
#include "params.h"
#include "my_string.h"
#include "my_fp.h"
#include "printf.h"
#include "param_save.h"
#include "errormessage.h"
#include "stm32_can.h"
#include "terminalcommands.h"

static void LoadDefaults(Terminal* term, char *arg);
static void StopInverter(Terminal* term, char *arg);
static void PrintErrors(Terminal* term, char *arg);

extern "C" const TERM_CMD TermCmds[] =
{
  { "set", TerminalCommands::ParamSet },
  { "get", TerminalCommands::ParamGet },
  { "flag", TerminalCommands::ParamFlag },
  { "stream", TerminalCommands::ParamStream },
  { "json", TerminalCommands::PrintParamsJson },
  { "can", TerminalCommands::MapCan },
  { "save", TerminalCommands::SaveParameters },
  { "load", TerminalCommands::LoadParameters },
  { "reset", TerminalCommands::Reset },
  { "defaults", LoadDefaults },
  { "stop", StopInverter },
  { "errors", PrintErrors },
  { NULL, NULL }
};

static void LoadDefaults(Terminal* term, char *arg)
{
   term = term;
   arg = arg;
   Param::LoadDefaults();
   printf("Defaults loaded\r\n");
}

static void StopInverter(Terminal* term, char *arg)
{
   term = term;
    arg = arg;
    Param::SetInt(Param::opmode, 0);
    printf("Inverter halted.\r\n");
}

static void PrintErrors(Terminal* term, char *arg)
{
   term = term;
   arg = arg;
   ErrorMessage::PrintAllErrors();
}
