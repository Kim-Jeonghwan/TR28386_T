/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_eQEP.c
    Description      : 
    Last Updated     : 2026. 04. 17.
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "CSU_eQEP.h"



/* ************************** [[   define   ]]  *********************************************************** */
#define GPIO_ZEROSETQEP_SWITCH         22u


/* ************************** [[   global   ]]  *********************************************************** */



/* ************************** [[  static prototype  ]]  *************************************************** */




/* ************************** [[  function  ]]  *********************************************************** */




void Init_Eqep1Gpio(void)
{
    EALLOW;

    // GPIO 20 : EQEP1A (A상) 설정
    GPIO_setPinConfig(GPIO_20_EQEP1_A);               // 20번 핀을 EQEP1A 기능으로 할당
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);          // 외부 인버터가 있으므로 내부 풀업 없이 표준 설정
    GPIO_setQualificationMode(20, GPIO_QUAL_6SAMPLE);     // 노이즈 제거를 위해 6샘플 필터링 적용

    // GPIO 21 : EQEP1B (B상) 설정
    GPIO_setPinConfig(GPIO_21_EQEP1_B);               // 21번 핀을 EQEP1B 기능으로 할당
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);          
    GPIO_setQualificationMode(21, GPIO_QUAL_6SAMPLE);     

    // HW SWITCH (GPIO65) - ZEROSETQEP SWITCH
    GPIO_SetupPinMux(GPIO_ZEROSETQEP_SWITCH, GPIO_MUX_CPU1, 0u);
    GPIO_SetupPinOptions(GPIO_ZEROSETQEP_SWITCH, GPIO_INPUT, GPIO_PULLUP);

    EDIS;
}

void Init_Eqep1(void)
{
// 1. eQEP1 모듈 클럭 활성화 (함수 내부에 포함)
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EQEP1); 

    // 2. 모듈 비활성화 후 설정 시작
    EQEP_disableModule(EQEP1_BASE);
    
    // 3. Quadrature 모드 및 해상도 설정
    EQEP_setDecoderConfig(EQEP1_BASE, (EQEP_CONFIG_QUADRATURE | 
                                       EQEP_CONFIG_2X_RESOLUTION | 
                                       EQEP_CONFIG_NO_SWAP));
    
    // 4. 포지션 카운터 최대치 설정 (누적 카운트용)
    EQEP_setPositionCounterConfig(EQEP1_BASE, EQEP_POSITION_RESET_MAX_POS, QEP_RESOLUTION_PER_REV - 1);
    // 5. 모듈 활성화
    EQEP_enableModule(EQEP1_BASE);
}


void EqeptoEncoder(void)
{
	// 1. 각도 계산
    xXmtSciPcMsg1.EncoderAngle = (uint16_t)(EQEP_getPosition(EQEP1_BASE) * (360.0f / 96.0f) * 100.0f + 0.5f); // +0.5f : 소수 1자리 반올림 위함
        
    // 2. RawPD 계산
    xXmtSciPcMsg1.EncoderRawPD = (uint16_t)EQEP_getPosition(EQEP1_BASE);
}


void updateHwSwitchStatus2(void)
{
    // GPIO 22번이 1(High)이면 eQEP 카운터 자체를 0으로 리셋
    if(GPIO_readPin(GPIO_ZEROSETQEP_SWITCH) == true)
    {
        EQEP_setPosition(EQEP1_BASE, 0u); // 하드웨어 카운터 레지스터를 0으로 설정
        
        // 구조체 값도 즉시 0으로 초기화
        xXmtSciPcMsg1.EncoderAngle = 0u;
        xXmtSciPcMsg1.EncoderRawPD = 0u;
    }
}
