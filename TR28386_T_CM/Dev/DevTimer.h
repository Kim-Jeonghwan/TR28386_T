/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevTimer.h
    Description      : CM Core SysTick Timer Header
    Last Updated     : 2026. 04. 22.
**********************************************************************/
#ifndef DEVTIMER_H
#define DEVTIMER_H

#include "main.h"


typedef struct
{
    uint16_t Cycle_1ms;
    uint16_t Cycle_10ms;
    uint16_t Cycle_100ms;
    uint16_t Cycle_1000ms;

    uint16_t Hzcnt;
    uint16_t Hz;
} stTimer;

extern stTimer xTimer;

// Timer Initialization
extern void Initial_TIMER(void);

// CPU Timer Interrupt Handlers
extern void isr_CpuTimer0(void); // Ethernet Tx
extern void isr_CpuTimer1(void); // Periodic Tasks
extern void isr_CpuTimer2(void); // Hz Measurement

#endif // DEVTIMER_H
