/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevIPC.c
    Description      : CM Core IPC Device Driver
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#include "DevIPC.h"

// CM 측 IPC 수신 인터럽트 서비스 루틴
// void isrIpcFromCPU1(void); 

void Initial_IPC(void)
{
    // SDK 파일(CM 코어용 driverlib)이 추가된 후 IPC 초기화 코드 구현
    /* 예시:
    IPC_clearFlagLtoR(IPC_CM_L_CPU1_R, IPC_FLAG_ALL);
    Interrupt_register(INT_IPC_0, isrIpcFromCPU1);
    */
}

void sendIpcMessageToCPU1(uint32_t command, uint32_t addr, uint32_t data)
{
    /* 예시:
    while(IPC_isFlagBusyLtoR(IPC_CM_L_CPU1_R, IPC_FLAG0));
    IPC_sendCommand(IPC_CM_L_CPU1_R, IPC_FLAG0, false, command, addr, data);
    */
}
