/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_IPC.c
    Description      : CM Core IPC 통신 프로토콜 정의
    Last Updated     : 2026. 04. 22.
**********************************************************************/

#include "CSU_IPC.h"

stRcvIpcMsg xRcvIpcCmMsg;
stXmtIpcMsg xXmtIpcCmMsg;

/**
 * @brief CM으로부터 수신된 IPC 메시지 처리 (이더넷 데이터 수신)
 * @param command 수신된 명령
 * @param addr 수신된 주소 (필요 시 데이터 포인터)
 * @param data 수신된 데이터
 */
void recvIpcCmMessage(uint32_t command, uint32_t addr, uint32_t data)
{
    xRcvIpcCmMsg.Command = (uint16_t)(command & 0xFFFFu);
    xRcvIpcCmMsg.Address = addr;
    xRcvIpcCmMsg.Data = data;

    // TODO: CM에서 받은 이더넷 데이터에 대한 처리 구현
    switch(xRcvIpcCmMsg.Command)
    {
        case 0x01u:
            // 예시 명령어 1 처리
            break;
            
        default:
            break;
    }
}

/**
 * @brief CM으로 보낼 데이터 전송 (이더넷 송신)
 */
void sendIpcCmMessage1(void)
{
    uint32_t sendCmd = xXmtIpcCmMsg.Command;
    uint32_t sendAddr = xXmtIpcCmMsg.Address;
    uint32_t sendData = xXmtIpcCmMsg.Data;
    
    sendIpcMessageToCM(sendCmd, sendAddr, sendData);
}
