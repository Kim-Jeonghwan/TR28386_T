/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_IPC.h
    Description      : CM Core IPC Communication Protocol Definition
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#ifndef CSU_IPC_H
#define CSU_IPC_H

#include "main.h"

typedef struct {
    uint16_t Command;
    uint32_t Address;
    uint32_t Data;
} stRcvIpcMsg;

typedef struct {
    uint16_t Command;
    uint32_t Address;
    uint32_t Data;
} stXmtIpcMsg;

extern stRcvIpcMsg xRcvIpcCmMsg;
extern stXmtIpcMsg xXmtIpcCmMsg;

// Process received IPC message from CM
void recvIpcCmMessage(uint32_t command, uint32_t addr, uint32_t data);

// Send message to CM
void sendIpcCmMessage1(void);

#endif // CSU_IPC_H
