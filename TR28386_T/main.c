/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : main.c
    Description      : 
    Last Updated     : 2026. 04. 17.
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"



/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */



/* ************************** [[  static prototype  ]]  *************************************************** */
// Cycle 1ms(1000Hz)
static void cycle_1ms(void);

// 10ms(100Hz)
static void cycle_10ms(void);

// 100ms(10Hz)
static void cycle_100ms(void);

// 1000ms(1Hz)
static void cycle_1000ms(void);



/* ************************** [[  function  ]]  *********************************************************** */
/*
@funtion	void main(void)
@brief		
@param		void
@return		void
@remark	
	-	
*/
void main(void)
{
	DSP_Initialization();

	// Idle(Background) Loop
	while(1u)
	{
		if(xTimer.Cycle_1ms >= 1u)
		{
			cycle_1ms();
			xTimer.Cycle_1ms = 0u;
		}

		if(xTimer.Cycle_10ms >= 10u)
		{
			cycle_10ms();
			xTimer.Cycle_10ms = 0u;
		}

		if(xTimer.Cycle_100ms >= 100u)
		{
			cycle_100ms();
			xTimer.Cycle_100ms = 0u;
		}

		if(xTimer.Cycle_1000ms >= 1000u)
		{
			cycle_1000ms();
			xTimer.Cycle_1000ms = 0u;
		}
	}
}



/*
@funtion	static void cycle_1ms(void)
@brief		1ms 마다 수행 하는 동작 
@param		void
@return		static void
@remark	
	-	
*/
static void cycle_1ms(void)
{
	// Create code
	xTimer.Hzcnt++;
}




/*
@funtion	static void cycle_10ms(void)
@brief		10ms 마다 수행 하는 동작 
@param		void
@return		static void
@remark	
	-	
*/
static void cycle_10ms(void)
{
	EqeptoEncoder();

	updateHwSwitchStatus2();
	updateTactStatus();

    updateAdcData();
    
    // EPWM7A 상태 업데이트 (10ms 주기)
    updateEpwm7aStatus();

    // 3. 통신 메시지 송신
    sendSciPcMessage1();

	// CAN 수신 처리 (폴링)
	recvCanMessage();

	// CAN 송신 데이터 최신화
	updateCanXmtData();

	// CAN 송신
	sendCanMessage();

    // CAN 상태 모니터링 (10ms 주기로 성공 여부 확인)
    updateCanStatus();

	updateEepromStatus();
}





/*
@funtion	static void cycle_100ms(void)
@brief		100ms 마다 수행 하는 동작 
@param		void
@return		static void
@remark	
	-	
*/
static void cycle_100ms(void)
{
    // 시스템 상태 LED 처리 (주기적 깜빡임 등을 제어하므로 100ms 유지)
    updateLedStatus();
}




/*
@funtion	static void cycle_1000ms(void)
@brief		1000ms 마다 수행 하는 동작 
@param		void
@return		static void
@remark	
	-	
*/
static void cycle_1000ms(void)
{


}

