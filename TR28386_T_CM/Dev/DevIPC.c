/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevIPC.c
    Description      : CM Core IPC Device Driver
    Last Updated     : 2026. 04. 30.
**********************************************************************/

#include "DevIPC.h"

static __interrupt void isrIpcFromCPU1(void);

void Initial_IPC(void)
{
    // 1. IPC 플래그 초기화
    IPC_clearFlagLtoR(IPC_CM_L_CPU1_R, IPC_FLAG_ALL);

    // 2. CPU1으로부터 수신받을 인터럽트 등록 (CPU1은 FLAG1을 사용하여 명령을 보냄)
    // CM 측에서 FLAG1에 대응하는 인터럽트 라인을 등록 (IPC_INT1 사용)
    IPC_registerInterrupt(IPC_CM_L_CPU1_R, IPC_INT1, isrIpcFromCPU1);
}

static __interrupt void isrIpcFromCPU1(void)
{
    uint32_t command, addr, data;
    bool status;

    // CPU1이 보낸 FLAG1 명령 읽기
    status = IPC_readCommand(IPC_CM_L_CPU1_R, IPC_FLAG1, IPC_ADDR_CORRECTION_DISABLE, &command, &addr, &data);

    if(status == true)
    {
        // CSU_IPC 핸들러 호출
        recvIpcCpu1Message(command, addr, data);

        // Acknowledge the flag (CPU1의 FLAG1 Busy 해제)
        IPC_ackFlagRtoL(IPC_CM_L_CPU1_R, IPC_FLAG1);
    }
}

void sendIpcMessageToCPU1(uint32_t command, uint32_t addr, uint32_t data)
{
    // CPU1은 FLAG0을 모니터링하여 수신함
    // FLAG0이 사용 중인지 확인
    while(IPC_isFlagBusyLtoR(IPC_CM_L_CPU1_R, IPC_FLAG0) == true)
    {
        // Wait
    }

    // Send the command using FLAG0
    IPC_sendCommand(IPC_CM_L_CPU1_R, IPC_FLAG0, IPC_ADDR_CORRECTION_DISABLE, command, addr, data);
}
