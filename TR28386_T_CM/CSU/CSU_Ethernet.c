/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_Ethernet.c
    Description      : Ethernet Protocol Handler
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#include "CSU_Ethernet.h"

void processReceivedEthernetPacket(uint8_t *packet, uint16_t length)
{
    // 수신한 이더넷 패킷(TCP/UDP) 페이로드 해석
    // 여기서 IPC를 통해 CPU1으로 데이터 전달 (sendIpcMessageToCPU1 호출)
}
