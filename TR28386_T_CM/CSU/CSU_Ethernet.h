/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_Ethernet.h
    Description      : Ethernet Protocol Handler
    Last Updated     : 2026. 04. 21.
**********************************************************************/

#ifndef CSU_ETHERNET_H
#define CSU_ETHERNET_H

#include "main.h"

void processReceivedEthernetPacket(uint8_t *packet, uint16_t length);

#endif // CSU_ETHERNET_H
