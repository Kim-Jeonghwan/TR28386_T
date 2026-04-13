/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : CSU_IPC.c
    Description      : PC Interface Communication (IPC) Protocol Definition
    Last Updated     : 2026. 04. 13.
**********************************************************************/

/* ************************** [[   include  ]]  *********************************************************** */
#include "CSU_IPC.h"



/* ************************** [[   define   ]]  *********************************************************** */
#define IPC_SOF		0x7Eu
#define IPC_EOT		0x0Du
#define IPC_MSG1	0x10u


/* ************************** [[   global   ]]  *********************************************************** */
stRcvIpcMsg1	xRcvIpcMsg1;
stXmtIpcMsg1	xXmtIpcMsg1;



/* ************************** [[  static prototype  ]]  *************************************************** */




/* ************************** [[  function  ]]  *********************************************************** */


/**
 * @brief PC로부터 수신된 IPC 메시지 해석 및 구조체 업데이트
 * @details ID 0x10 패킷을 수신하여 시퀀스 번호, 제어 명령(Command), EEPROM 주소 및 데이터를 파싱합니다.
 * @param ID 수신된 메시지의 식별 번호 (0x10u)
 * @param Data[] 수신된 데이터 배열 (바이트 단위)
 */
void recvIpcMessage(uint16_t ID, uint16_t Data[])
{
    volatile uint16_t pos = 0u;

//    onConv32 on32;
    onConv16 on16;
    
    switch(ID)
    {
    case 0x10u:
        /* 1. Sequence Number 저장 (1 byte) */
        xRcvIpcMsg1.IncNumber = (uint16_t)(Data[pos++] & 0xFFu);

        /* 2. Command 저장 (2 bytes: Command(1) + Reserved(1)) */
        on16.byte.B0 = (uint8_t)Data[pos++]; // Row 5: Command
        on16.byte.B1 = (uint8_t)Data[pos++]; // Row 6: Reserved
        xRcvIpcMsg1.Command.all = on16.u16;

        /* 3. EepAddr 저장 (2 bytes, Little Endian) */
        on16.byte.B0 = (uint8_t)Data[pos++]; // Row 7
        on16.byte.B1 = (uint8_t)Data[pos++]; // Row 8
        xRcvIpcMsg1.EepAddr = on16.u16;

        /* 4. EepromWriteVal 저장 (1 byte) */
        xRcvIpcMsg1.EepromWriteVal = (uint16_t)(Data[pos++] & 0xFFu); // Row 9

        /* 5. Epwm7aDuty & Freq 저장 */
        xRcvIpcMsg1.Epwm7aDuty = (uint16_t)(Data[pos++] & 0xFFu); // Row 10
        xRcvIpcMsg1.Epwm7aFreq = (uint16_t)(Data[pos++] & 0xFFu); // Row 11

    break;
    
    default:
    break;
    }
}


/**
 * @brief 엔코더 상태 및 데이터를 PC로 전송 (10ms 주기)
 * @details 전체 16바이트 패킷을 구성하며, 순수 데이터 길이는 10바이트입니다.
 * 패킷 구조: SOF(1) + ID(1) + LEN(1) + DATA(10) + Checksum(1) + EOT(1)
 */
void sendIpcMessage1(void)
{
    volatile uint16_t pos = 0u;
//    onConv32 on32;
    onConv16 on16;
    uint16_t i = 0u;
    uint16_t Buf[25u] = {0u, }; 
    uint16_t CheckSum = 0u;

    /* 1. 헤더 구성 */
    Buf[pos++] = IPC_SOF;           // Buf[0]: 0x7E
    Buf[pos++] = IPC_MSG1;          // Buf[1]: 0x10 (ID)
    Buf[pos++] = 0u;                // Buf[2]: Length (데이터 개수 11이 들어갈 자리)
    
    /* 2. Sequence Number */
    Buf[pos++] = (uint16_t)(xXmtIpcMsg1.IncNumber++ & 0xFFu); // Buf[3]

    /* 3. Status Bit Fields 직접 조립 (가장 안전한 방법) */
    on16.u16 = 0u; // 필수: 쓰레기 값 방지
    on16.u16 |= (xXmtIpcMsg1.Tact01 == true ? 1u : 0u) << 0u; // D0
    on16.u16 |= (xXmtIpcMsg1.Tact02 == true ? 1u : 0u) << 1u; // D1
    // Reserved 는 자동으로 0
    Buf[pos++] = (on16.u16 & 0xFFu);

    /* 4. EncoderAngle (uint16_t - 2 bytes, Little Endian) */
    on16.u16 = xXmtIpcMsg1.EncoderAngle;
    Buf[pos++] = on16.byte.B0; // Low Byte
    Buf[pos++] = on16.byte.B1; // High Byte

    /* 5. EncoderRawPD (1 byte) + EepromReadVal (1 byte) */
    Buf[pos++] = (uint16_t)(xXmtIpcMsg1.EncoderRawPD & 0xFFu);
    Buf[pos++] = (uint16_t)(xXmtIpcMsg1.EepromReadVal & 0xFFu);

    /* 6. 나머지 16비트 데이터들 (각 2 bytes, Little Endian) */
    // PWMRaw
    on16.u16 = xXmtIpcMsg1.PWMRaw;
    Buf[pos++] = on16.byte.B0; 
    Buf[pos++] = on16.byte.B1;

    // PWMRCLPF
    on16.u16 = xXmtIpcMsg1.PWMRCLPF;
    Buf[pos++] = on16.byte.B0; 
    Buf[pos++] = on16.byte.B1;

    // PWMBWLPF
    on16.u16 = xXmtIpcMsg1.PWMBWLPF;
    Buf[pos++] = on16.byte.B0; 
    Buf[pos++] = on16.byte.B1;

    // PotenRAW
    on16.u16 = xXmtIpcMsg1.PotenRAW;
    Buf[pos++] = on16.byte.B0; 
    Buf[pos++] = on16.byte.B1;

    // PotenMAVE
    on16.u16 = xXmtIpcMsg1.PotenMAVE;
    Buf[pos++] = on16.byte.B0; 
    Buf[pos++] = on16.byte.B1;

    /* 6. 데이터 길이(LEN) 계산 및 업데이트 */
    Buf[2] = (uint16_t)(pos - 2u);

    /* 7. CheckSum 계산 (Length인 Buf[2]부터 데이터 끝인 Buf[pos-1]까지) */
    CheckSum = 0u;
    for(i = 2u; i < pos; i++)
    {
        CheckSum += (Buf[i] & 0xFFu);
    }

    Buf[pos++] = (uint16_t)(CheckSum & 0xFFu); // Buf[19]
    Buf[pos++] = IPC_EOT;                   // Buf[20]: 0x0D


    /* 8. 최종 전송 (pos는 현재 21) */
    xmtScib_IPC(Buf, pos);
}
