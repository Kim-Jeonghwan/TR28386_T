/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_EPWM.c
    Description      : EPWM 7A Control
    Last Updated     : 2026. 04. 13.
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "CSU_EPWM.h"


/* ************************** [[   define   ]]  *********************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */
// 이전 설정 상태 보관
static uint16_t prevFreq = 0xFFFF;
static uint16_t prevDuty = 0xFFFF;
static bool prevEn = false;


/* ************************** [[  function  ]]  *********************************************************** */

/**
 * @brief EPWM7A(GPIO12) 핀 설정
 */
static void initEpwm7aGpio(void)
{
    EALLOW;
    GPIO_setPinConfig(GPIO_12_EPWM7A);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    EDIS;
}

/**
 * @brief EPWM7A 초기화 (기본: 100Hz, 50% Duty, 출력 정지)
 */
void Initial_Epwm7a(void)
{
    initEpwm7aGpio();

    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM7 = 1; // ePWM7 클럭 활성화
    EDIS;

    // 기본 설정: Up-count mode
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm7Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm7Regs.EPWMSYNCOUTEN.all = SYNC_OUT_SRC_DISABLE_ALL; // 동기화 비활성화
    EPwm7Regs.TBCTR = 0x0000;

    // 액션 한정기 설정
    EPwm7Regs.AQCTLA.bit.CAU = AQ_CLEAR; // CMPA 도달 시 Low
    EPwm7Regs.AQCTLA.bit.ZRO = AQ_SET;   // 0 도달 시 High
    
    // PWM 출력 오프 (Force Low)로 시작
    EPwm7Regs.AQCSFRC.bit.CSFA = 1;
    
    // 초기 구조체 설정: 꺼짐, Duty 50, Freq 1 (100Hz)
    xRcvIpcMsg1.Command.bit.Epwm7aEn = 0;
    xRcvIpcMsg1.Epwm7aDuty = 50u;
    xRcvIpcMsg1.Epwm7aFreq = 1u;

    // 클럭 동기화 (ePWM7 활성화)
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; 
    EDIS;
}

/**
 * @brief IPC 메시지에 따른 EPWM7A 상태 및 주파수, Duty 업데이트
 */
void updateEpwm7aStatus(void)
{
    if (xRcvIpcMsg1.Command.bit.Epwm7aEn) 
    {
        if (!prevEn) 
        {
            // Enable output (해제)
            EPwm7Regs.AQCSFRC.bit.CSFA = 0; // Resume normal PWM
        }
        
        if ((xRcvIpcMsg1.Epwm7aFreq != prevFreq) || (xRcvIpcMsg1.Epwm7aDuty != prevDuty)) 
        {
            uint32_t prd = 0;
            uint16_t hspclkdiv = 0;
            uint16_t clkdiv = 0;

            switch (xRcvIpcMsg1.Epwm7aFreq) {
                case 0: // 10Hz
                    hspclkdiv = 7; // /14
                    clkdiv = 7;    // /128
                    prd = 11160;
                    break;
                case 1: // 100Hz
                    hspclkdiv = 0; // /1
                    clkdiv = 7;    // /128
                    prd = 15624;
                    break;
                case 2: // 1kHz
                    hspclkdiv = 0; // /1
                    clkdiv = 3;    // /8
                    prd = 24999;
                    break;
                case 3: // 10kHz
                    hspclkdiv = 0; // /1
                    clkdiv = 0;    // /1
                    prd = 19999;
                    break;
                case 4: // 100kHz
                    hspclkdiv = 0; // /1
                    clkdiv = 0;    // /1
                    prd = 1999;
                    break;
                case 5: // 1MHz
                    hspclkdiv = 0; // /1
                    clkdiv = 0;    // /1
                    prd = 199;
                    break;
                case 6: // 10MHz
                    hspclkdiv = 0; // /1
                    clkdiv = 0;    // /1
                    prd = 19;
                    break;
                default:
                    hspclkdiv = 0; // /1
                    clkdiv = 7;    // /128
                    prd = 15624;   // Default 100Hz
                    break;
            }

            EPwm7Regs.TBCTL.bit.HSPCLKDIV = hspclkdiv;
            EPwm7Regs.TBCTL.bit.CLKDIV = clkdiv;
            EPwm7Regs.TBPRD = (uint16_t)prd;
            
            // Duty calculation (1-100%)
            uint16_t duty = xRcvIpcMsg1.Epwm7aDuty;
            if (duty > 100) duty = 100;
            if (duty < 1)   duty = 1;
            
            // CAU=AQ_CLEAR, ZRO=AQ_SET 이므로, CMPA 시간(0~CMPA)이 High 유지 시간.
            uint32_t cmpa = ((prd + 1) * duty) / 100;
            if (cmpa > prd) cmpa = prd + 1; // 100% duty (항상 켜져있음)
            
            EPwm7Regs.CMPA.bit.CMPA = (uint16_t)cmpa;

            prevFreq = xRcvIpcMsg1.Epwm7aFreq;
            prevDuty = xRcvIpcMsg1.Epwm7aDuty;
        }
    } 
    else 
    {
        if (prevEn) 
        {
            EPwm7Regs.AQCSFRC.bit.CSFA = 1; // Force Low
        }
    }
    prevEn = xRcvIpcMsg1.Command.bit.Epwm7aEn;
}
