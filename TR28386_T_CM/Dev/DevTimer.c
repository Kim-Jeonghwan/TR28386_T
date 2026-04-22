/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevTimer.c
    Description      : CM Core CPU Timer 0 Source
    Last Updated     : 2026. 04. 22.
**********************************************************************/

#include "DevTimer.h"

stTimer xTimer;

void Initial_TIMER(void)
{
    // 타이머 변수 초기화
    (void)memset(&xTimer, 0, sizeof(xTimer));

    // --- CPU Timer 0 초기화 (이더넷 송신용: 1ms) ---
    CPUTimer_setPeriod(CPUTIMER0_BASE, 125000U); // 1ms @ 125MHz
    CPUTimer_setPreScaler(CPUTIMER0_BASE, 0U);
    CPUTimer_stopTimer(CPUTIMER0_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);
    Interrupt_registerHandler(INT_TIMER0, isr_CpuTimer0);
    CPUTimer_enableInterrupt(CPUTIMER0_BASE);
    CPUTimer_startTimer(CPUTIMER0_BASE);

    // --- CPU Timer 1 초기화 (주기적 작업용: 1ms) ---
    CPUTimer_setPeriod(CPUTIMER1_BASE, 125000U); // 1ms @ 125MHz
    CPUTimer_setPreScaler(CPUTIMER1_BASE, 0U);
    CPUTimer_stopTimer(CPUTIMER1_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
    Interrupt_registerHandler(INT_TIMER1, isr_CpuTimer1);
    CPUTimer_enableInterrupt(CPUTIMER1_BASE);
    CPUTimer_startTimer(CPUTIMER1_BASE);

    // --- CPU Timer 2 초기화 (Hz 측정용: 1000ms = 1s) ---
    CPUTimer_setPeriod(CPUTIMER2_BASE, 125000000U); // 1s @ 125MHz
    CPUTimer_setPreScaler(CPUTIMER2_BASE, 0U);
    CPUTimer_stopTimer(CPUTIMER2_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER2_BASE);
    Interrupt_registerHandler(INT_TIMER2, isr_CpuTimer2);
    CPUTimer_enableInterrupt(CPUTIMER2_BASE);
    CPUTimer_startTimer(CPUTIMER2_BASE);
}

// Timer 0: 이더넷 송신 핸들러 (1ms)
void isr_CpuTimer0(void)
{
    // TODO: 이더넷 송신 관련 동작 추가
}

// Timer 1: 주기적 작업 핸들러 (1ms)
void isr_CpuTimer1(void)
{
    xTimer.Cycle_1ms++;
    xTimer.Cycle_10ms++;
    xTimer.Cycle_100ms++;
    xTimer.Cycle_1000ms++;
}

// Timer 2: Hz 측정 핸들러 (1000ms = 1s)
void isr_CpuTimer2(void)
{
    xTimer.Hz = xTimer.Hzcnt;
    xTimer.Hzcnt = 0;
}
