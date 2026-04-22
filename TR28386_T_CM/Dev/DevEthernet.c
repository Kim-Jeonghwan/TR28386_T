/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : DevEthernet.c
    Description      : Ethernet Device Driver
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#include "DevEthernet.h"

void Initial_Ethernet(void)
{
    // MAC 하드웨어 초기화 (EMAC), PHY 칩 설정, lwIP 스택 초기화
}

void updateEthernetTask(void)
{
    // 패킷 수신 대기 및 lwIP 타이머 처리
}
