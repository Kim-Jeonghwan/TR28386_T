/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_Ethernet.c
    Description      : Ethernet Protocol Handler
    Last Updated     : 2026. 04. 30.
**********************************************************************/

#include "CSU_Ethernet.h"

void sendEthernetLoopbackPacket(uint8_t command)
{
    // 코딩 트레이닝을 위한 이더넷 루프백 테스트용 패킷 생성
    uint8_t txBuf[64] = {0};
    
    // Dummy MAC Header
    txBuf[0] = 0xFF; txBuf[1] = 0xFF; txBuf[2] = 0xFF; txBuf[3] = 0xFF; txBuf[4] = 0xFF; txBuf[5] = 0xFF; // Dest MAC
    txBuf[6] = 0x00; txBuf[7] = 0x11; txBuf[8] = 0x22; txBuf[9] = 0x33; txBuf[10] = 0x44; txBuf[11] = 0x55; // Src MAC
    txBuf[12] = 0x88; txBuf[13] = 0xB5; // EtherType (Local Experimental)
    
    // Payload (Command)
    txBuf[14] = command;
    
    // 원래는 EMAC 드라이버의 송신 함수(예: Ethernet_sendPacket)를 호출해야 하지만,
    // 이더넷 하드웨어가 완전히 초기화되지 않았을 수 있으므로 소프트웨어적으로 루프백(수신 함수 직접 호출) 처리합니다.
    processReceivedEthernetPacket(txBuf, 64);
}

void processReceivedEthernetPacket(uint8_t *packet, uint16_t length)
{
    // 수신한 이더넷 패킷(TCP/UDP) 페이로드 해석
    // 여기서 IPC를 통해 CPU1으로 데이터 전달 (sendIpcMessageToCPU1 호출)
    
    if (length >= 15 && packet[12] == 0x88 && packet[13] == 0xB5)
    {
        // 루프백 테스트 패킷 감지
        uint8_t cmd = packet[14];
        
        if (cmd >= 0x10 && cmd <= 0x13)
        {
            // 수신된 명령을 CPU1에 전달하기 위해 +0x10 오프셋을 더하여 전송 (예: 0x10 -> 0x20)
            sendIpcMessageToCPU1((uint32_t)cmd + 0x10, 0, 0);
        }
    }
}
