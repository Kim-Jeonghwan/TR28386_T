/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevIPC.c
    Description      : CM Core IPC Device Driver
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#include "DevIPC.h"

static __interrupt void isrIpcFromCM(void);

void Initial_IPC(void)
{
    // Clear any IPC flags
    IPC_clearFlagLtoR(IPC_CPU1_L_CM_R, IPC_FLAG_ALL);

    // Register IPC Interrupt for receiving from CM (IPC0)
    IPC_registerInterrupt(IPC_CPU1_L_CM_R, IPC_INT0, isrIpcFromCM);

    // Synchronize with CM core (Optional, can be used to wait until CM is ready)
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
