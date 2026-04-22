/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : main.c
    Description      : CM Core Main Entry
    Last Updated     : 2026. 04. 22.
**********************************************************************/

#include "main.h"

// --- Static Function Prototypes ---
static void Cycle_1ms(void);
static void Cycle_10ms(void);
static void Cycle_100ms(void);
static void Cycle_1000ms(void);

int main(void)
{
    /* 1. 시스템 초기화 (CM 코어 클럭 및 인터럽트 등) */
    CM_init(); 

    /* 2. 통신 및 주변장치 초기화 */
    Initial_IPC();
    Initial_Ethernet();
    Initial_TIMER();
    
    /* 2.5 전역 인터럽트 활성화 */
    (void)Interrupt_enableInProcessor(); 

    /* 3. 무한 루프 (Background Loop) */
    while(1)
    {
        /* --- 1ms Task --- */
        if (xTimer.Cycle_1ms >= 1)
        {
            xTimer.Cycle_1ms = 0;
            Cycle_1ms();
        }

        /* --- 10ms Task --- */
        if (xTimer.Cycle_10ms >= 10)
        {
            xTimer.Cycle_10ms = 0;
            Cycle_10ms();
        }

        /* --- 100ms Task --- */
        if (xTimer.Cycle_100ms >= 100)
        {
            xTimer.Cycle_100ms = 0;
            Cycle_100ms();
        }

        /* --- 1000ms Task --- */
        if (xTimer.Cycle_1000ms >= 1000)
        {
            xTimer.Cycle_1000ms = 0;
            Cycle_1000ms();
        }

        // 이더넷 패킷 처리 (상시 백그라운드 폴링)
        updateEthernetTask();
    }
}

// --- Periodic Task Implementations ---

static void Cycle_1ms(void)
{
    // 1ms 작업 내용
    xTimer.Hzcnt++;
}

static void Cycle_10ms(void)
{
    static uint16_t u16ToggleCnt = 0;

    // CM 코어는 카운터를 항상 유지하며, CPU1이 권한을 넘겨준 순간부터 실제 핀에 반영됩니다.
    u16ToggleCnt++;

    // 10ms * 20 = 200ms (0.2초)
    if (u16ToggleCnt >= 20)
    {
        u16ToggleCnt = 0;
        
        // GPIO 31번 상태 반전
        GPIO_togglePin(31);
    }
}

static void Cycle_100ms(void)
{
    // 100ms 작업 내용
}

static void Cycle_1000ms(void)
{
    // 1000ms 작업 내용
}
