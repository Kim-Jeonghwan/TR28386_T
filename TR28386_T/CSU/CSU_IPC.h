/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_IPC.h
    Description      : CM Core IPC 통신 프로토콜 정의
    Last Updated     : 2026. 04. 22.
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

// CM으로부터 수신된 IPC 메시지 처리
void recvIpcCmMessage(uint32_t command, uint32_t addr, uint32_t data);

// CM으로 메시지 전송
void sendIpcCmMessage1(void);

#endif // CSU_IPC_H
