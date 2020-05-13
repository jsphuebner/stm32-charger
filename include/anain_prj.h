#ifndef ANAIN_PRJ_H_INCLUDED
#define ANAIN_PRJ_H_INCLUDED

#include "hwdefs.h"

#define NUM_SAMPLES 12
#define SAMPLE_TIME ADC_SMPR_SMP_7DOT5CYC

#define ANA_IN_LIST \
   ANA_IN_ENTRY(uoutp,     GPIOC, 1) \
   ANA_IN_ENTRY(uoutn,     GPIOC, 0) \
   ANA_IN_ENTRY(idc,       GPIOA, 5) \
   ANA_IN_ENTRY(udc,       GPIOC, 3) \
   ANA_IN_ENTRY(tmpm,      GPIOC, 2) \
   ANA_IN_ENTRY(tmphs,     GPIOC, 4) \
   ANA_IN_ENTRY(uaux,      GPIOA, 3) \

#endif // ANAIN_PRJ_H_INCLUDED
