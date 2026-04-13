/**********************************************************************

	Nexcom Co., Ltd.
	Copyright 2021. All Rights Reserved.

	Filename		: DevTimer.c
	Version			: 00.10
	Description		: 
	Tracebility		: 
	Programmer	    :
	Last Updated	: 2025. 08. 04.

**********************************************************************/

/*
 * Modification History
 * --------------------
 * 
 * 
*/


/* ************************** [[   include  ]]  *********************************************************** */
#include "DevTimer.h"


/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */
stTimer xTimer;


/* ************************** [[  static prototype  ]]  *************************************************** */
static void initCPUTimers(void);

static void configCPUTimer(uint32_t cpuTimer, float32_t freq, float32_t period);


/* ************************** [[  function  ]]  *********************************************************** */
/*
@funtion	void Initial_TIMER(void)
@brief		
@param		void
@return		void
@remark	
	-	
*/
void Initial_TIMER(void)
{
	//	  
	// ISRs for each CPU Timer interrupt	 
	//    
	Interrupt_register(INT_TIMER0, &isr_CpuTimer0);
    Interrupt_register(INT_TIMER1, &isr_CpuTimer1);
    Interrupt_register(INT_TIMER2, &isr_CpuTimer2);

	//
	// Initializes the Device Peripheral. For this example, only initialize the
	// Cpu Timers.
	//
	initCPUTimers();

	//
	// Configure CPU-Timer 0, 1, and 2 to interrupt every 1, 2, 4 seconds:
	// 1, 2, 4 Period respectively (in uSeconds)
	//
	configCPUTimer(CPUTIMER0_BASE, DEVICE_SYSCLK_FREQ, 100);	 	// 100 us
	configCPUTimer(CPUTIMER1_BASE, DEVICE_SYSCLK_FREQ, 1000);		// 1 ms
	configCPUTimer(CPUTIMER2_BASE, DEVICE_SYSCLK_FREQ, 1000000);	// 1000 ms


	//
	// Enables CPU int1, int13, and int14 which are connected to CPU-Timer 0,
	// CPU-Timer 1, and CPU-Timer 2 respectively.
	// Enable TINT0 in the PIE: Group 1 interrupt 7
	//
	Interrupt_enable(INT_TIMER0);
	Interrupt_enable(INT_TIMER1);
	Interrupt_enable(INT_TIMER2);

	//
	// Starts CPU-Timer 0, CPU-Timer 1, and CPU-Timer 2.
	//
	CPUTimer_startTimer(CPUTIMER0_BASE);
	CPUTimer_startTimer(CPUTIMER1_BASE);
	CPUTimer_startTimer(CPUTIMER2_BASE);


}

//
// initCPUTimers - This function initializes all three CPU timers
// to a known state.
//
static void initCPUTimers(void)
{
    //
    // Initialize timer period to maximum
    //
    CPUTimer_setPeriod(CPUTIMER0_BASE, 0xFFFFFFFF);
    CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF);
    CPUTimer_setPeriod(CPUTIMER2_BASE, 0xFFFFFFFF);

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    //
    CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);
    CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);
    CPUTimer_setPreScaler(CPUTIMER2_BASE, 0);

    //
    // Make sure timer is stopped
    //
    CPUTimer_stopTimer(CPUTIMER0_BASE);
    CPUTimer_stopTimer(CPUTIMER1_BASE);
    CPUTimer_stopTimer(CPUTIMER2_BASE);

    //
    // Reload all counter register with period value
    //
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER2_BASE);

    //
    // Reset interrupt counter
    //
	(void)memset(&xTimer, 	0, sizeof(xTimer));		//  Ÿ�̸� ���� �ʱ�ȭ
}



//
// configCPUTimer - This function initializes the selected timer to the
// period specified by the "freq" and "period" parameters. The "freq" is
// entered as Hz and the period in uSeconds. The timer is held in the stopped
// state after configuration.
//
static void configCPUTimer(uint32_t cpuTimer, float32_t freq, float32_t period)
{
    uint32_t temp;

    //
    // Initialize timer period:
    //
    temp = (uint32_t)((freq / 1000000) * period);
    CPUTimer_setPeriod(cpuTimer, temp - 1);

    //
    // Set pre-scale counter to divide by 1 (SYSCLKOUT):
    //
    CPUTimer_setPreScaler(cpuTimer, 0);

    //
    // Initializes timer control register. The timer is stopped, reloaded,
    // free run disabled, and interrupt enabled.
    // Additionally, the free and soft bits are set
    //
    CPUTimer_stopTimer(cpuTimer);
    CPUTimer_reloadTimerCounter(cpuTimer);
    CPUTimer_setEmulationMode(cpuTimer,
                              CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_enableInterrupt(cpuTimer);
}





/*
@funtion	__interrupt void isr_CpuTimer0(void)
@brief		100 us
@param		void
@return		__interrupt void
@remark	
	-	
*/
__interrupt void isr_CpuTimer0(void)
{
    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //

    sendScib_IPC();
    
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


/*
@funtion	__interrupt void isr_CpuTimer1(void)
@brief		1 ms
@param		void
@return		__interrupt void
@remark	
	-	
*/
__interrupt void isr_CpuTimer1(void)
{
	xTimer.Cycle_1ms ++;
	xTimer.Cycle_10ms ++;
	xTimer.Cycle_100ms ++;
	xTimer.Cycle_1000ms ++;

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}



/*
@funtion	__interrupt void isr_CpuTimer2(void)
@brief		1000 ms
@param		void
@return		__interrupt void
@remark	
	-	
*/
__interrupt void isr_CpuTimer2(void)
{
    //
    // The CPU acknowledges the interrupt.
    //

    xTimer.Hz = xTimer.Hzcnt;
    xTimer.Hzcnt = 0u;

	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}



