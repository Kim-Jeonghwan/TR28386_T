/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevIPC.c
    Description      : CM Core IPC Device Driver 및 공유 메모리 설정
    Last Updated     : 2026. 05. 06.
**********************************************************************/

#include "DevIPC.h"

static __interrupt void isrIpcFromCM(void);

// 1. CM 코어가 사용할 RAM 권한을 부여하는 함수
void Initial_IPC_Mastership(void)
{
    // F2838x에서는 각 RAM 섹션당 2비트를 사용하여 마스터를 지정함 (10b = CM)
    EALLOW;
    // Shared RAM (S0~S3) 권한을 CM으로 설정 (0xAA = 10101010b -> S0,S1,S2,S3 모두 CM)
    HWREG(MEMCFG_BASE + MEMCFG_O_LSXMSEL) = (HWREG(MEMCFG_BASE + MEMCFG_O_LSXMSEL) & ~0x00FFU) | 0x00AAU;

    // GSRAM (GS0~GS1) 권한을 CM으로 설정 (0x0A = 1010b -> GS0, GS1 모두 CM)
    HWREG(MEMCFG_BASE + MEMCFG_O_GSXMSEL) = (HWREG(MEMCFG_BASE + MEMCFG_O_GSXMSEL) & ~0x000FU) | 0x000AU;
    EDIS;
}

// 2. IPC 통신 설정 및 CM 코어와의 동기화 함수
void Initial_IPC(void)
{
    // CM으로부터 수신받을 인터럽트 등록 (IPC0)
    IPC_registerInterrupt(IPC_CPU1_L_CM_R, IPC_INT0, isrIpcFromCM);

    // Synchronize with CM core
    IPC_sync(IPC_CPU1_L_CM_R, IPC_FLAG31);
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
