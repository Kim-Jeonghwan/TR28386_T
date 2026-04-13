/**********************************************************************

	Nexcom Co., Ltd.
	Copyright 2021. All Rights Reserved.

	Filename		: DevDspInit.c
	Version			: 00.00
	Description		: 
	Tracebility		: 
	Programmer	    :
	Last Updated	: 2026. 04. 13.

**********************************************************************/

/*
 * Modification History
 * --------------------
 * 
 * 
*/


/* ************************** [[   include  ]]  *********************************************************** */
#include "DevDspInit.h"

/* ************************** [[   define   ]]  *********************************************************** */


/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[  static prototype  ]]  *************************************************** */
static void Initial_GPIO(void);

static void Init_GpioDin(void);

static void Init_GpioDout(void);

static void InitialPeripherals(void);


/* ************************** [[  function  ]]  *********************************************************** */
/*
@funtion	void DSP_Initialization(void)
@brief		DSP 초기화 
@param		void
@return		void
@remark	
	-	
*/
void DSP_Initialization(void)
{
	// 시스템 및 주변회로 클럭 설정
	Device_init();

	Initial_GPIO();

	// 주변회로 인터럽트 확장 회로(PIE) 및 관련 레지스터 초기화 / CPU 인터럽트 비-활성화
	Interrupt_initModule();

	// PIE 벡터 테이블 초기화 및 기본 인터럽트 서비스 루틴 연결
	Interrupt_initVectorTable();


	InitialPeripherals();

	// 실시간 디버깅 활성화, 전역 인터럽트 스위치 ON
	ERTM;	// Debug Enable Mask 비트 설정 (실시간 디버깅이 가능하도록 ST1 레지스터의 /DBGM 비트를 0으로 클리어)
	EINT;	// 전역 인터럽트 스위치 ON (/INTM ON)
}






/*
@funtion	static void Initial_GPIO(void)
@brief		GPIO 초기화(DIN or DOUT)
@param		void
@return		static void
@remark	
	-	CAN, SPI, SCI, I2C GPIO는 각각  설정함 
*/

static void Initial_GPIO(void)
{
	Init_GpioDin();
	
	Init_GpioDout();
}

/*
@funtion	static void Init_GpioDin(void)
@brief		
@param		void
@return		static void
@remark	
	-	
*/
static void Init_GpioDin(void)
{
    initGpioDinTact();
}



/*
@funtion	static void Init_GpioDout(void)
@brief		
@param		void
@return		static void
@remark	
	-	
*/
static void Init_GpioDout(void)
{
	initGpioDoutLed();

}



/*
@funtion	static void DevInitPeripherals(void)
@brief		DSP 주변 디바이스 초기화 설정
@param		void
@return		static void
@remark	
	-	
*/
static void InitialPeripherals(void)
{
	InitialAdc();
	initEPWM8();
	Initial_Epwm7a();
    Initial_Adc();
	
	Initial_SPI();

	Initial_SCI();

	Initial_TIMER();

	Initial_LED();

	Init_Eqep1Gpio();
	Init_Eqep1();

	EEPROM_Init();
	Initial_Tact();


}



