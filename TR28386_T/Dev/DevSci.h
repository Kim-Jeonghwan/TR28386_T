/**********************************************************************

	Nexcom Co., Ltd.
	Copyright 2021. All Rights Reserved.

	Filename		: DevSci.h
	Version			: 00.00
	Description		: 
	Tracebility		: 
	Programmer	    :
	Last Updated	: 2026. 02. 02.

**********************************************************************/

/*
 * Modification History
 * --------------------
 * 
 * 
*/


#ifndef DEVSCI_H
#define DEVSCI_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main.h"


/* ************************** [[   define   ]]  *********************************************************** */
#define QUEUE_MAX_SCI 200u


/* ************************** [[   enum or struct   ]]  *************************************************** */
typedef enum
{
	eSciB_SOF = 0,
	eSciB_MSGID,
	eSciB_LEN,
	eSciB_DATA,
	eSciB_CRC,
	eSciB_EOT
}eSciB;

typedef struct
{
    eSciB           Frame;      /* 프레임 수신 위치 (SOF, ID, DATA 등 상태 제어) */
    uint16_t          MSGID;      /* 메시지 식별 ID (예: 0x10) */
    uint16_t          LEN;        /* 메시지 데이터 길이 (Payload Length) */
    uint16_t          DATA[50u];  /* 실제 수신 데이터 버퍼 */
    uint16_t          CRC;        /* 수신된 체크섬/CRC 값 */
    uint16_t 			POS;        /* 현재 데이터 수신/파싱 인덱스 위치 */
} stSciB;



typedef struct
{
    uint16_t front;
    uint16_t rear;
    uint16_t Data[QUEUE_MAX_SCI];
} stQsci;






/* ************************** [[   global   ]]  *********************************************************** */


/* ************************** [[  function  ]]  *********************************************************** */
void Initial_SCI(void);



__interrupt void isrScib_IPC(void);

void xmtScib_IPC(uint16_t data[], uint16_t len);

void sendScib_IPC(void);



#endif	// #ifndef DEVSCI_H


