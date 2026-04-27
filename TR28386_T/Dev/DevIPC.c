/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevIPC.c
    Description      : CM Core IPC Device Driver 및 공유 메모리 설정
    Last Updated     : 2026. 04. 23.
**********************************************************************/

#include "DevIPC.h"

static __interrupt void isrIpcFromCM(void);

void Initial_IPC(void)
{
    // 1. GSRAM 권한 설정 (CM이 GSRAM 0에 쓸 수 있도록 설정)
    // F2838x에서는 GSxMSEL 레지스터가 각 GSRAM당 2비트를 사용하여 마스터를 지정합니다.
    // 00b: CPU1, 01b: CPU2, 10b: CM
    // 현재 프로젝트의 Driverlib이 1비트 방식(F2837xD 호환)으로 구현되어 있어 직접 레지스터를 설정합니다.
    EALLOW;
    HWREG(MEMCFG_BASE + MEMCFG_O_GSXMSEL) = (HWREG(MEMCFG_BASE + MEMCFG_O_GSXMSEL) & ~0x0003U) | 0x0002U;
    EDIS;

    // 2. IPC 플래그 초기화
    IPC_clearFlagLtoR(IPC_CPU1_L_CM_R, IPC_FLAG_ALL);

    // 3. CM으로부터 수신받을 인터럽트 등록 (IPC0)
    IPC_registerInterrupt(IPC_CPU1_L_CM_R, IPC_INT0, isrIpcFromCM);

    // Synchronize with CM core (Optional)
    // IPC_sync(IPC_CPU1_L_CM_R, IPC_FLAG31);
}

static __interrupt void isrIpcFromCM(void)
{
    uint32_t command, addr, data;
    bool status;

    // Read the command
    status = IPC_readCommand(IPC_CPU1_L_CM_R, IPC_FLAG0, IPC_ADDR_CORRECTION_DISABLE, &command, &addr, &data);

    if(status == true)
    {
        // Process message in CSU_IPC
        recvIpcCmMessage(command, addr, data);

        // Acknowledge the flag
        IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG0);
    }

    // Clear PIE ACK for IPC (CM-to-CPU IPC0 is in Group 11)
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

void sendIpcMessageToCM(uint32_t command, uint32_t addr, uint32_t data)
{
    // Wait for the IPC flag to be available
    while(IPC_isFlagBusyLtoR(IPC_CPU1_L_CM_R, IPC_FLAG1) == true)
    {
        // Do nothing, wait
    }

    // Send the command
    IPC_sendCommand(IPC_CPU1_L_CM_R, IPC_FLAG1, IPC_ADDR_CORRECTION_DISABLE, command, addr, data);
}
