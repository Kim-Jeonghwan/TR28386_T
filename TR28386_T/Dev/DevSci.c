/**********************************************************************

	Nexcom Co., Ltd.
	Copyright 2021. All Rights Reserved.

	Filename		: DevSci.c
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


/* ************************** [[   include  ]]  *********************************************************** */
#include "DevSci.h"


/* ************************** [[   define   ]]  *********************************************************** */
#define IPC_GPIO_PIN_SCIB_RXD	87u             // GPIO number for SCI RX
#define IPC_GPIO_PIN_SCIB_TXD	86u             // GPIO number for SCI TX
#define IPC_GPIO_CFG_SCIB_RXD	GPIO_87_SCIB_RX	// "pinConfig" for SCI RX
#define IPC_GPIO_CFG_SCIB_TXD	GPIO_86_SCIB_TX	// "pinConfig" for SCI TX



/* ************************** [[   global   ]]  *********************************************************** */
static stQsci	xQueIPC;


/* ************************** [[  static prototype  ]]  *************************************************** */
static void initScib_IPC(void);


static void enqueueSci(stQsci *pstQ, uint16_t Data);

static uint16_t dequeueSci(stQsci *pstQ, uint16_t *pData);


/* ************************** [[  function  ]]  *********************************************************** */
/*
@funtion	void Initial_SCI(void)
@brief		
@param		void
@return		void
@remark	
	-	
*/
void Initial_SCI(void)
{
	initScib_IPC();

	memset(&xQueIPC, 0u, sizeof(xQueIPC));
}


/*
@funtion	static void initScib_IPC(void)
@brief		
@param		void
@return		static void
@remark	
	-	
*/
static void initScib_IPC(void)
{
	//
	// Initialize the Device Peripherals:
	//

	//
	// GPIO87 is the SCI Rx pin.
	//
	GPIO_setControllerCore(IPC_GPIO_PIN_SCIB_RXD, GPIO_CORE_CPU1);
	GPIO_setPinConfig(IPC_GPIO_CFG_SCIB_RXD);
	GPIO_setDirectionMode(IPC_GPIO_PIN_SCIB_RXD, GPIO_DIR_MODE_IN);
	GPIO_setPadConfig(IPC_GPIO_PIN_SCIB_RXD, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(IPC_GPIO_PIN_SCIB_RXD, GPIO_QUAL_ASYNC);

	//
	// GPIO86 is the SCI Tx pin.
	//
	GPIO_setControllerCore(IPC_GPIO_PIN_SCIB_TXD, GPIO_CORE_CPU1);
	GPIO_setPinConfig(IPC_GPIO_CFG_SCIB_TXD);
	GPIO_setDirectionMode(IPC_GPIO_PIN_SCIB_TXD, GPIO_DIR_MODE_OUT);
	GPIO_setPadConfig(IPC_GPIO_PIN_SCIB_TXD, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(IPC_GPIO_PIN_SCIB_TXD, GPIO_QUAL_ASYNC);


	//
	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	//
	Interrupt_register(INT_SCIB_RX, isrScib_IPC);

	Interrupt_enable(INT_SCIB_RX);
	
	Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);


    //
    // 8 char bits, 1 stop bit, no parity. Baud rate is 115200.
    //
    SCI_setConfig(SCIB_BASE, DEVICE_LSPCLK_FREQ, 115200u, (SCI_CONFIG_WLEN_8 |
                                                         SCI_CONFIG_STOP_ONE |
                                                         SCI_CONFIG_PAR_NONE));
    SCI_enableModule(SCIB_BASE);
    SCI_resetChannels(SCIB_BASE);
    SCI_enableFIFO(SCIB_BASE);

    //
    // RX and TX FIFO Interrupts Enabled
    //
    SCI_enableInterrupt(SCIB_BASE, SCI_INT_RXFF);
    SCI_disableInterrupt(SCIB_BASE, SCI_INT_RXERR);

    //
    // The transmit FIFO generates an interrupt when FIFO status
    // bits are less than or equal to 2 out of 16 words
    // The receive FIFO generates an interrupt when FIFO status
    // bits are greater than equal to 2 out of 16 words
    //
    SCI_setFIFOInterruptLevel(SCIB_BASE, SCI_FIFO_TX1, SCI_FIFO_RX1);
    SCI_performSoftwareReset(SCIB_BASE);

    SCI_resetTxFIFO(SCIB_BASE);
    SCI_resetRxFIFO(SCIB_BASE);

}




/*
@funtion	static interrupt void isrScib_IPC(void)
@brief		
@param		void
@return		[ static interrupt void ]	
@remark	
	-	
*/
__interrupt void isrScib_IPC(void)
{
	static stSciB	xRcvIPC;

    uint16_t Data[1u];

	// FIFO에 데이터가 있을 때만 루프를 돌며 읽는 것이 안전하지만, 
    // 현재 레벨이 1이므로 1바이트씩 처리하는 로직을 유지
    SCI_readCharArray(SCIB_BASE, Data, 1u);

  	switch(xRcvIPC.Frame)
  	{
  	case eSciB_SOF:
  		if(Data[0u] == 0x7Eu)
  		{
  			xRcvIPC.Frame	= eSciB_MSGID;
  			xRcvIPC.POS	= 0u;
  			xRcvIPC.CRC	= 0u;
  		}
  	break;

  	case eSciB_MSGID:
		xRcvIPC.MSGID	= Data[0u];
		xRcvIPC.Frame	= eSciB_LEN;
  	break;

  	case eSciB_LEN:
		xRcvIPC.LEN	= Data[0u];
		xRcvIPC.CRC	= Data[0u];			// LEN 포함 합산
		xRcvIPC.POS = 0u;				// POS 초기화 보장
        
        if(xRcvIPC.LEN > 0u)
        {
            xRcvIPC.LEN--;              // 보정: LEN 필드가 자기 자신을 포함하므로 실제 데이터는 LEN-1개
            xRcvIPC.Frame = eSciB_DATA;
        }
        else
        {
            xRcvIPC.Frame = eSciB_CRC;   // 데이터가 없는 경우
        }
  	break;

  	case eSciB_DATA:
		xRcvIPC.DATA[xRcvIPC.POS++] = (Data[0u] & 0x00FFu); // 하위 8비트만 명시
		xRcvIPC.CRC += (Data[0u] & 0x00FFu);
		xRcvIPC.LEN--;                          // 남은 개수 하나 감소
        if(xRcvIPC.LEN == 0u)                   // 다 받았으면 CRC 단계로
        {
            xRcvIPC.Frame = eSciB_CRC;
        }	
  	break;

  	case eSciB_CRC:
  		if((xRcvIPC.CRC & 0xFFu) == Data[0u])
  		{
			xRcvIPC.Frame	= eSciB_EOT;	// 맞으면 EOT 대기
		}
		else
		{
			xRcvIPC.Frame	= eSciB_SOF;	// 틀리면 SOF 로
		}
  	break;  	

  	case eSciB_EOT:
		if(Data[0u] == 0x0Du)	// 마지막 바이트 0x0D 인지 체크
		{
			recvIpcMessage(xRcvIPC.MSGID, xRcvIPC.DATA);
		}
		xRcvIPC.Frame	= eSciB_SOF;	// 성공하든 실패하든 초기화
  	break;  	
  	
  	default:
  		xRcvIPC.Frame = eSciB_SOF;
  	break;
  	}

    SCI_clearOverflowStatus(SCIB_BASE);

    SCI_clearInterruptStatus(SCIB_BASE, SCI_INT_RXFF);

    //
    // Issue PIE ack
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);

}


/*
@funtion	void xmtScib_IPC(uint16_t data[], uint16_t len)
@brief		
@param		[ uint16_t data[] ]	
@param		[ uint16_t len ]	
@return		void
@remark	
	-	
*/
void xmtScib_IPC(uint16_t data[], uint16_t len)
{
#if 1 // 2025-08-05 9:13:57
	uint16_t i = 0u;

	for(i = 0u; i < len; i++)
	{
		enqueueSci(&xQueIPC, data[i]);
	}
#else
	SCI_writeCharArray(SCIB_BASE, data, len);
#endif // #if 0 // 2025-08-05 9:13:57
}



/*
@funtion	void sendScib_IPC(void)
@brief		100 us  ?         
@param		void
@return		void
@remark	
	-	ref isr_CpuTimer0()
*/
void sendScib_IPC(void)
{
    uint16_t i = 0u;
    uint16_t len = 0u;
    uint16_t popData = 0u;
    uint16_t sendData[20u] = {0u};		// 10 에서 20으로 변경

    for(i = 0u; i < 20u; i++)
    {
        if(dequeueSci(&xQueIPC, &popData) == 1u)
        {
            sendData[len ++] = popData;
        }
    }

    if(len > 0u)
    {
        SCI_writeCharArray(SCIB_BASE, sendData, len);
    }
}




/*
@funtion	static void enqueueSci(stQsci *pstQ, uint16_t Data)
@brief		
@param		[ stQsci *pstQ ]	
@param		[ uint16_t Data ]	
@return		static void
@remark	
	-	
*/
static void enqueueSci(stQsci *pstQ, uint16_t Data)
{
    uint16_t nRear = 0u;

    if(pstQ->rear < QUEUE_MAX_SCI)
    {
        nRear = ((pstQ->rear + 1u) % QUEUE_MAX_SCI);

        if(nRear != pstQ->front)
        {
            pstQ->Data[pstQ->rear] = Data;
            pstQ->rear = nRear;
        }
    }
}




/*
@funtion	static uint16_t dequeueSci(stQsci *pstQ, uint16_t *pData)
@brief		
@param		[ stQsci *pstQ ]	
@param		[ uint16_t *pData ]	
@return		static uint16_t
@remark	
	-	
*/
static uint16_t dequeueSci(stQsci *pstQ, uint16_t *pData)
{
    uint16_t result = 0u;

    if(pstQ->front != pstQ->rear)
    {
        *pData = pstQ->Data[pstQ->front];

        if(pstQ->front <= QUEUE_MAX_SCI)
        {
            pstQ->front = (pstQ->front + 1u) % QUEUE_MAX_SCI;
        }

        result = 1u;
    }

    return result;
}

