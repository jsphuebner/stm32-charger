#ifndef PinMode_PRJ_H_INCLUDED
#define PinMode_PRJ_H_INCLUDED

#include "hwdefs.h"


#define DIG_IO_LIST \
    DIG_IO_ENTRY(dcsw_out,    GPIOC, GPIO13, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(outc_out,    GPIOB, GPIO1,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(led_out,     GPIOC, GPIO12, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(start_in,    GPIOB, GPIO6,  PinMode::INPUT_PD)    \
    DIG_IO_ENTRY(emcystop_in, GPIOC, GPIO7,  PinMode::INPUT_PU)    \
    DIG_IO_ENTRY(bk_in,       GPIOB, GPIO12, PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(bms_in,      GPIOC, GPIO8,  PinMode::INPUT_PU)    \

#endif // PinMode_PRJ_H_INCLUDED
