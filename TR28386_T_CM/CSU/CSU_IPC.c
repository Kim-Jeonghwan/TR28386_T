/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_IPC.c
    Description      : IPC Protocol (CM to CPU1)
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#include "CSU_IPC.h"

void recvIpcCpu1Message(uint32_t command, uint32_t addr, uint32_t data)
{
    // CPU1에서 전송한 IPC 메시지 파싱
    // 예: 이더넷으로 특정 패킷 송신 요청 등
}
