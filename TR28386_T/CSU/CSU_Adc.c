/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_Adc.c
    Description      : ADC Application Logic (100ms Periodic Task)
    Last Updated     : 2026. 04. 13.
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "CSU_Adc.h"

/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[  function  ]]  *********************************************************** */

/**
 * @brief ADC 애플리케이션 초기화
 */
void Initial_Adc(void)
{
    xXmtIpcMsg1.PotenRAW = 0u;
    xXmtIpcMsg1.PotenMAVE = 0u;
    xXmtIpcMsg1.PWMRaw = 0u;
    xXmtIpcMsg1.PWMRCLPF = 0u;
    xXmtIpcMsg1.PWMBWLPF = 0u;
}

/**
 * @brief 100ms 주기로 실행되는 ADC 데이터 처리 루틴
 * @details ISR에서 고속(10kHz)으로 샘플링된 전압 값을 가져와 1000배 스케일링하여 IPC 구조체에 반영합니다.
 */
void updateAdcData(void)
{
    // 소수점 3자리 표기를 위해 1000 곱함 (반올림 포함)
    // ISR(AdcaIsr)에서 실시간으로 업데이트되는 float 전역 변수 사용
    xXmtIpcMsg1.PotenRAW  = (uint16_t)(f32PotenRAW * 1000.0f + 0.5f);
    xXmtIpcMsg1.PWMRaw    = (uint16_t)(f32PWMRaw * 1000.0f + 0.5f);
    xXmtIpcMsg1.PWMRCLPF  = (uint16_t)(f32PWMRCLPF * 1000.0f + 0.5f);
    xXmtIpcMsg1.PWMBWLPF  = (uint16_t)(f32PWMBWLPF * 1000.0f + 0.5f);

    // Poten MAVE (AdcaIsr에서 계산된 float 값을 변환)
    xXmtIpcMsg1.PotenMAVE = (uint16_t)(f32PotenMAVE * 1000.0f + 0.5f);
}


