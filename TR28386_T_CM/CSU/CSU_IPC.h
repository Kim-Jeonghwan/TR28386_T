/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_IPC.h
    Description      : IPC Protocol (CM to CPU1)
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#ifndef CSU_IPC_H
#define CSU_IPC_H

#include "main.h"

void recvIpcCpu1Message(uint32_t command, uint32_t addr, uint32_t data);

#endif // CSU_IPC_H
