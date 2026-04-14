/**********************************************************************

	Nexcom Co., Ltd.
	Copyright 2021. All Rights Reserved.

	Filename		: DevAdc.c
	Version			: 00.00
	Description		: ADC Driver (Focus on ADCINA2)
	Tracebility		: 
	Programmer		: 
	Last Updated	: 2026. 04. 13.

	Function List	:	
						

**********************************************************************/

/*
 * Modification History
 * --------------------
 * 
 * 
*/


/* DESCRIPTION
 * 
 * 
*/


/* ************************** [[   include  ]]  *********************************************************** */
#include "DevAdc.h"
#include "CSU_Adc.h"

/* ************************** [[   define   ]]  *********************************************************** */
#define DEFAULT_MAVE_COUNT   100u   // 이동 평균 필터 카운트
#define DEFAULT_PWM_HZ       100000u  // ePWM8 트리거 주파수 (100kHz 조정)


/* ************************** [[   global   ]]  *********************************************************** */
float32_t lpf_PrevValue   = 0.0f;     // ADC LPF 이전 값
float32_t lpf_PrevPWM7a   = 0.0f;     // ADC LPF 이전 값 (PWM7a)
float32_t f32PotenRAW     = 0.0f;     // Poten RAW 결과 (float) 
float32_t f32PotenMAVE    = 0.0f;     // Poten MAVE 결과 (float)
float32_t f32PWMRaw       = 0.0f;     // PWM Raw 결과 (float)
float32_t f32PWMRCLPF     = 0.0f;     // PWM RC LPF 결과 (float)
float32_t f32PWMBWLPF     = 0.0f;     // PWM BW LPF 결과 (float)
float32_t f32PotenSum      = 0.0f;     // Poten 누적 합 (Running Sum용)


uint16_t  ResultsIndex    = 0u;       // ADC 결과 인덱스

float32_t AdcResults[BUFF_SIZE];      // ADC 결과 저장 버퍼

/* ************************** [[  function  ]]  *********************************************************** */

// FLASH 메모리에서 실행할 경우, 인터럽트 서비스 루틴을 램(RAM) 영역에서 실행하기 위한 설정
#ifdef _FLASH
    #pragma CODE_SECTION(AdcaIsr, ".TI.ramfunc");
#endif


/*
@funtion    interrupt void AdcaIsr(void)
@brief      ADC-A 인터럽트 서비스 루틴 (ADCINA2 사용)
@param      void
@return     interrupt void
@remark
    - 10kHz 주기로 실행 (ePWM8 트리거)
*/
interrupt void AdcaIsr(void)
{
    // 1. 실시간 전압 판독 및 저장 (10kHz)
    f32PotenRAW  = (float32_t)AdcaResultRegs.ADCRESULT0 * CONV_ADC_3_3V; // A2
    f32PWMRaw    = (float32_t)AdcaResultRegs.ADCRESULT1 * CONV_ADC_3_3V; // A4
    f32PWMRCLPF  = (float32_t)AdccResultRegs.ADCRESULT0 * CONV_ADC_3_3V; // C4
    f32PWMBWLPF  = (float32_t)AdcdResultRegs.ADCRESULT0 * CONV_ADC_3_3V; // D4

    xTimer.Hzcnt++;

    // 2. Poten MAVE 최적화 로직 (Running Sum 방식 - O(1))
    // 루프를 돌지 않고 가장 오래된 값을 빼고 새 값을 더해 효율성 극대화
    f32PotenSum -= AdcResults[ResultsIndex];
    f32PotenSum += f32PotenRAW;
    AdcResults[ResultsIndex++] = f32PotenRAW;

    if(ResultsIndex >= DEFAULT_MAVE_COUNT)
    {
        ResultsIndex = 0u;
    }

    // 평균값 업데이트 (단일 나눗셈만 수행)
    f32PotenMAVE = f32PotenSum / (float32_t)DEFAULT_MAVE_COUNT;


    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      // ADC-A 인터럽트 플래그(ADCINT1) 클리어

    // ADCINT1 이벤트 플래그 오버플로우 확인 및 클리어
    if(AdcaRegs.ADCINTOVF.bit.ADCINT1 == 1u)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1u; // 인터럽트 오버플로우 플래그 클리어
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1u; // 인터럽트 플래그 재클리어
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;     // PIE 인터럽트 승인 (Group 1)
}



/*
@funtion    void InitialAdc(void)
@brief      ADC 초기화 시작
@param      void
@return     void
@remark
    -
*/
void InitialAdc(void)
{
    InitAdcModules();       // ADC 모듈 하드웨어 초기화

    EALLOW;
    PieVectTable.ADCA1_INT = &AdcaIsr; // AdcaIsr 함수를 인터럽트 벡터 테이블에 등록
    EDIS;

    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;  // PIE 영역에서 ADCA1 인터럽트 활성화
    IER |= M_INT1;                      // CPU 인터럽트 라인 1 활성화 (ADC-A)
}


/*
@funtion    void InitAdcModules(void)
@brief      ADC 모듈 초기 설정 (ADCINA2 전용 설정)
@param      void
@return     void
@remark
    -
*/
void InitAdcModules(void)
{
    // ADC-A 초기화
	EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE		= 6u;	// ADCCLK = SYSCLK / 4 (200MHz / 4 = 50MHz : 최대 속도)
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS 	= 1u;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ 		= 1u;
    DELAY_US(1000);
    
	AdcaRegs.ADCSOC0CTL.bit.CHSEL		= 2u;
	AdcaRegs.ADCSOC0CTL.bit.ACQPS		= 14u;	// 샘플링 윈도우 최소화 (고속 대응)
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL		= 19u;

	AdcaRegs.ADCSOC1CTL.bit.CHSEL		= 4u;
	AdcaRegs.ADCSOC1CTL.bit.ACQPS		= 14u;
	AdcaRegs.ADCSOC1CTL.bit.TRIGSEL		= 19u;

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL 	= 0u; 	// SOC0 변환 완료 시 INT1 시퀀스 발생
    AdcaRegs.ADCINTSEL1N2.bit.INT1E 	= 1u; 	// ADCINT1 인터럽트 활성화
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 	= 1u; 	// 인터럽트 플래그 클리어
    EDIS;

	// ADC-B 초기화
	EALLOW;
    AdcbRegs.ADCCTL2.bit.PRESCALE		= 14u;
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ 		= 1u;
    DELAY_US(1000);
	EDIS;


	// ADC-C 초기화
	EALLOW;
    AdccRegs.ADCCTL2.bit.PRESCALE		= 6u;
    AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdccRegs.ADCCTL1.bit.ADCPWDNZ 		= 1u;
    DELAY_US(1000);

	AdccRegs.ADCSOC0CTL.bit.CHSEL		= 4u;
	AdccRegs.ADCSOC0CTL.bit.ACQPS		= 14u;
	AdccRegs.ADCSOC0CTL.bit.TRIGSEL		= 19u;
	EDIS;


	// ADC-D 초기화
	EALLOW;
    AdcdRegs.ADCCTL2.bit.PRESCALE		= 6u;
    AdcSetMode(ADC_ADCD, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcdRegs.ADCCTL1.bit.ADCPWDNZ 		= 1u;
    DELAY_US(1000);

	AdcdRegs.ADCSOC0CTL.bit.CHSEL		= 4u;
	AdcdRegs.ADCSOC0CTL.bit.ACQPS		= 14u;
	AdcdRegs.ADCSOC0CTL.bit.TRIGSEL		= 19u;
    EDIS;
}



/*
@funtion    float32_t low_pass_filter(float32_t input, float32_t alpha, float32_t *prev_output)
@brief      로우 패스 필터 (LPF) 구현
@param      float32_t input: 입력 값
@param      float32_t alpha: 필터 계수 (0~1)
@param      float32_t *prev_output: 이전 출력 값 저장 포인터
@return     float32_t: 필터링된 결과 값
@remark
    -
*/
float32_t low_pass_filter(float32_t input, float32_t alpha, float32_t *prev_output)
{
    float32_t output = alpha * input + (1.0f - alpha) * (*prev_output);
    *prev_output = output;
    return output;
}

/*
@funtion    float32_t Within_f32(float32_t val, float32_t min, float32_t max)
@brief      값의 범위를 제한 (Saturation)
@param      float32_t val: 입력 값
@param      float32_t min: 최솟값
@param      float32_t max: 최댓값
@return     float32_t: 제한된 범위 내의 결과
@remark
    -
*/
float32_t Within_f32(float32_t val, float32_t min, float32_t max)
{
    if (val <= min) return min;
    if (val >= max) return max;
    return val;
}


/*
@funtion    void initEPWM8(void)
@brief      ePWM8 모듈 초기화 (ADC 트리거 용도)
@param      void
@return     void
@remark
    -
*/
void initEPWM8(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM8 = 1u; // ePWM8 클럭 활성화
    
    // ePWM8 기본 설정
    EPwm8Regs.TBPRD = SYSCLK / ((uint32_t)DEFAULT_PWM_HZ * 4u); // 주기 설정 (예: 500 타임베이스 = 100kHz)
    EPwm8Regs.TBPHS.bit.TBPHS = 0x0000u; // 위상 0 설정
    EPwm8Regs.TBCTR = 0x0000u;          // 타임베이스 카운터 초기화

    EPwm8Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // 타임베이스 카운트 모드: 업 카운트
    EPwm8Regs.TBCTL.bit.PHSEN = TB_DISABLE;    // 위상 로드 비활성화
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;   // 고속 타임베이스 클럭 분주 (1)
    EPwm8Regs.TBCTL.bit.CLKDIV = TB_DIV2;      // 타임베이스 클럭 분주 (2)

    // 비교값 설정
    EPwm8Regs.CMPA.bit.CMPA = EPwm8Regs.TBPRD / 2u; // 50% 듀티 사이클 설정

    // ePWM 액션 한정기(Action Qualifier) 설정
    EPwm8Regs.AQCTLA.bit.CAU = AQ_SET;   // CMPA와 일치하면 출력 High
    EPwm8Regs.AQCTLA.bit.ZRO = AQ_CLEAR; // 카운터가 0이면 출력 Low
    
    // ADC 트리거 설정 (SOCA)
    EPwm8Regs.ETSEL.bit.SOCAEN = 1u;    // SOCA 활성화
    EPwm8Regs.ETSEL.bit.SOCASEL = 2u;   // 카운터가 주기(TBPRD)에 도달할 때 SOCA 발생
    EPwm8Regs.ETPS.bit.SOCAPRD = 1u;    // 첫 번째 이벤트마다 SOCA 발생
    
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1u; // ePWM 타임베이스 클럭 동기화 (카운터 시작)
    EDIS;
}
