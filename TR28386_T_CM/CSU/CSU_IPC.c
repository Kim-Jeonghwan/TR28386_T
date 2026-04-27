/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_IPC.c
    Description      : IPC Protocol (CM to CPU1) 구현 및 이더넷 공유 메모리 처리
    Last Updated     : 2026. 04. 23.
**********************************************************************/

#include "CSU_IPC.h"

// Message RAM 영역에 구조체 포인터 할당
volatile stIpcDataPacket *pxIpcCpu1ToCm = (volatile stIpcDataPacket *)IPC_CPU1_TO_CM_MSGRAM_ADDR;
volatile stIpcDataPacket *pxIpcCmToCpu1 = (volatile stIpcDataPacket *)IPC_CM_TO_CPU1_MSGRAM_ADDR;

// GSRAM0 영역에 이더넷 수신용 버퍼 할당
#pragma DATA_SECTION(g_xEthRcvBuffer, "gs0_section")
stEthPacketBuffer g_xEthRcvBuffer;

void recvIpcCpu1Message(uint32_t command, uint32_t addr, uint32_t data)
{
    if(command == IPC_CMD_ETH_XMT_REQUEST)
    {
        // addr는 CPU1이 보낸 GSRAM1의 CM 주소
        stEthPacketBuffer *pXmtData = (stEthPacketBuffer *)addr;
        
        // TODO: 실제 이더넷 드라이버를 통해 데이터 전송
        // 예: Ethernet_sendPacket(pXmtData->Data, pXmtData->Length);
    }
}

/**
 * @brief 공유 메모리(Message RAM)를 통한 대용량 데이터 처리 예시
 * 이 함수는 IPC Flag0 인터럽트 서비스 루틴(ISR) 내에서 호출될 수 있습니다.
 */
void processBulkDataFromCPU1(void)
{
    uint32_t cmd = pxIpcCpu1ToCm->Command;
    uint32_t status = pxIpcCpu1ToCm->Status;
    
    if(status == 0x01) // Data Ready
    {
        // pxIpcCpu1ToCm->Payload 데이터를 읽어 이더넷 등으로 전송 처리
        // ... (이더넷 송신 로직)
        
        // 처리가 완료되었음을 표시 (필요 시)
        pxIpcCpu1ToCm->Status = 0x00; // Done
    }
}

/**
 * @brief CM이 이더넷 패킷 수신 시 호출하여 CPU1에게 알림
 * @param pData 수신된 패킷 데이터 포인터
 * @param len 패킷 길이
 */
void notifyEthPacketToCPU1(uint8_t *pData, uint16_t len)
{
    if(len > 1514) len = 1514;

    // 1. GSRAM 버퍼에 데이터 복사
    g_xEthRcvBuffer.Length = (uint32_t)len;
    memcpy(g_xEthRcvBuffer.Data, pData, len);

    // 2. 현재 버퍼의 주소를 CPU1이 이해할 수 있는 주소로 변환
    uint32_t cpuAddr = CONVERT_CM_TO_CPU_ADDR((uint32_t)&g_xEthRcvBuffer);

    // 3. IPC를 통해 명령어와 주소 전달 (IPC_FLAG1 사용)
    // DevIPC에 정의된 전송 함수 호출 또는 직접 IPC_sendCommand 호출
    sendIpcMessageToCPU1(IPC_CMD_ETH_RCV_NOTIFY, cpuAddr, 0);
}
