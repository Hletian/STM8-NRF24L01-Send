/******************** (C) COPYRIGHT  ���iCreateǶ��ʽ���������� **************
 * �ļ���  ��nRF24L01.c
 * ����    ��nRF24L01����ģ��ʵ��   
 * ʵ��ƽ̨�����STM8������
 * ��汾  ��V2.1.0
 * ����    �����  QQ��779814207
 * ����    ��
 * �Ա�    ��http://shop71177993.taobao.com/
 * �޸�ʱ�� ��2012-12-23

  ���STM8������Ӳ������
    |----------------------------|
	|  CE  - PI0             	 |
	|  SPI_CLK - PC5(SPI+SCK)	 |
    |  SPI_MOSI - PC6(SPI_MOSI)  |
    |  SPI_MISO - PC7(SPI_MISO)  |
	|  CSN - PC1             	 |
	|  IRQ - PC2             	 |
    |----------------------------|

*******************************************************************************/

/* ����ϵͳͷ�ļ� */

/* �����Զ���ͷ�ļ� */
#include "stm8s.h"
#include "uart1.h"
#include "nRF24L01.h"

/* �Զ��������� */

/* �Զ���� */

/* ȫ�ֱ������� */

u8  TxAddr[]={0x34,0x43,0x10,0x10,0x01};//���͵�ַ

/* һ���򵥵���ʱ���� */
void Delay(u32 cnt)
{
	u32 i;
	
	for (; cnt > 0; cnt--) {
		for (i = 0; i < 3000; i++)
			;
	}
}

/*******************************************************************************
 * ����: NRF24L01_Pin_Conf
 * ����: nRF24L01�������ų�ʼ����SPIģ���ʼ��
 * �β�: ��
 * ����: ��
 * ˵��: �� 
 ******************************************************************************/
void nRF24L01_Pin_Conf()
{
	/* ��ʼ��SPI--��λ��ǰ��6M, ��ģʽ��SCK����Ϊ�͵�ƽ����һ�����زɼ����� */
	SPI_Init(SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_4, SPI_MODE_MASTER,\
				SPI_CLOCKPOLARITY_LOW, SPI_CLOCKPHASE_1EDGE, \
				SPI_DATADIRECTION_2LINES_FULLDUPLEX, SPI_NSS_SOFT, 0x07);
	
	SPI_Cmd(ENABLE);	/* ʹ��SPI */
	
	GPIO_Init(GPIOC, CSN, GPIO_MODE_OUT_PP_HIGH_FAST);	/* CSN���ų�ʼ��Ϊ��� */
	GPIO_Init(GPIOI, CE, GPIO_MODE_OUT_PP_HIGH_FAST);	/* CE���ų�ʼ��Ϊ��� */
}


/*******************************************************************************
 * ����: nRF24L01_SPI_RW
 * ����: nRF24L01 SPIģʽ�¶�д��������
 * �β�: data -> ҪдnRF24L01������
 * ����: nRF24L01���ص�����
 * ˵��: �� 
 ******************************************************************************/
static u8 nRF24L01_SPI_RW(u8 date)
{
	/* �ȴ�DR�Ĵ����ǿ� */
	while (SPI_GetFlagStatus( SPI_FLAG_TXE) == RESET);
	
	/* ͨ��SPI����һ���ֽ� */
	SPI_SendData(date);
	
	/* �ȴ�����һ���ֽ� */
	while (SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET);
	
	/* ����SPI�����ϵ��ֽ� */
	return SPI_ReceiveData();  
}

/*******************************************************************************
 * ����: nRF24L01_Read_Reg
 * ����: ��ȡnRF24L01�Ĵ�������
 * �β�: RegAddr -> nRF24L01�Ĵ�����ַ
 * ����: nRF24L01���ص�����
 * ˵��: �� 
 ******************************************************************************/
static u8 nRF24L01_Read_Reg(u8 RegAddr)
{
   u8 BackData;
   
   ClrCSN;							/* ����ʱ�� */
   nRF24L01_SPI_RW(RegAddr);		/* д�Ĵ�����ַ */
   BackData = nRF24L01_SPI_RW(0x00);	/* д����Ĵ���ָ�� */ 
   SetCSN;
   
   return(BackData); 				/* ����״ֵ̬ */
}

/*******************************************************************************
 * ����: nRF24L01_Write_Reg
 * ����: ��nRF24L01�Ĵ���д���ݺ���
 * �β�: RegAddr -> nRF24L01�Ĵ�����ַ
 *		 data -> д��Ĵ���������
 * ����: nRF24L01���ص�����
 * ˵��: �� 
 ******************************************************************************/
static u8 nRF24L01_Write_Reg(u8 RegAddr,u8 data)
{
   u8 BackData;
   
   ClrCSN;						/* ����ʱ�� */
   BackData = nRF24L01_SPI_RW(RegAddr);	/* д���ַ */
   nRF24L01_SPI_RW(data);				/* д��ֵ */
   SetCSN;
   
   return(BackData);
}

/*******************************************************************************
 * ����: nRF24L01_Write_Reg
 * ����: ��ȡnRF24L01RX FIFO�����ݺ���
 * �β�: RegAddr -> nRF24L01�Ĵ�����ַ
 *		 Rxdata -> ָ������������ָ��
 * 	     DataLen -> ���ݳ���
 * ����: nRF24L01���ص�����
 * ˵��: ����ģʽ��ʹ��
 ******************************************************************************/
static u8 nRF24L01_Read_RxData(u8 RegAddr, u8 *RxData, u8 DataLen)
{ 
	u8 BackData,i;
	
	ClrCSN;									/* ����ʱ�� */
	BackData = nRF24L01_SPI_RW(RegAddr);	/* д��Ҫ��ȡ�ļĴ�����ַ */
	for(i = 0; i < DataLen; i++) 			/* ��ȡ���� */
	{
		RxData[i] = nRF24L01_SPI_RW(0);
	} 
	
	SetCSN;
	
	return(BackData); 
}

/*******************************************************************************
 * ����: nRF24L01_Write_TxData
 * ����: д��nRF24L01TX FIFO�����ݺ���
 * �β�: RegAddr -> nRF24L01�Ĵ�����ַ
 *		 Rxdata -> ָ��Ҫд�����ݵ�ָ��
 * 	     DataLen -> ���ݳ���
 * ����: nRF24L01���ص�����
 * ˵��: ����ģʽ��ʹ��
 ******************************************************************************/
static u8 nRF24L01_Write_TxData(u8 RegAddr,u8 *TxData,u8 DataLen)
{ 
	u8 BackData,i;
	
	ClrCSN;
	BackData = nRF24L01_SPI_RW(RegAddr);	/* д��Ҫд��Ĵ����ĵ�ַ */
	for(i = 0; i < DataLen; i++)			/* д������ */
	{
		nRF24L01_SPI_RW(*TxData++);
	}   
	
	SetCSN;
	return(BackData);
}

/*******************************************************************************
 * ����: nRF24L01_Set_TxMode
 * ����: ��nRF24L01����Ϊ����ģʽ
 * �β�: ��
 * ����: ��
 * ˵��: 
 ******************************************************************************/
void nRF24L01_Set_TxMode()
{
    ClrCE; 
	
	/* ���÷��͵�ַ�͵�ַ���� */
   	nRF24L01_Write_TxData(W_REGISTER+TX_ADDR,TxAddr,TX_ADDR_WITDH);	
	/* Ϊ��Ӧ������豸������ͨ��0��ַ�ͷ��͵�ַ��ͬ */
	nRF24L01_Write_TxData(W_REGISTER+RX_ADDR_P0,TxAddr,TX_ADDR_WITDH);	

	/******�����йؼĴ�������**************/
  	nRF24L01_Write_Reg(W_REGISTER+EN_AA,0x01);       /* ʹ�ܽ���ͨ��0�Զ�Ӧ�� */
  	nRF24L01_Write_Reg(W_REGISTER+EN_RXADDR,0x01);   /* ʹ�ܽ���ͨ��0 */
   	nRF24L01_Write_Reg(W_REGISTER+SETUP_RETR,0x0a);  /* �Զ��ط���ʱ�ȴ�250us+86us���Զ��ط�10�� */
  	nRF24L01_Write_Reg(W_REGISTER+RF_CH,0x40);       /* ѡ����Ƶͨ��0x40 */
  	nRF24L01_Write_Reg(W_REGISTER+RF_SETUP,0x07);    /* ���ݴ�����1Mbps�����书��0dBm���������Ŵ������� */
	nRF24L01_Write_Reg(W_REGISTER+CONFIG,0x0e);      /* CRCʹ�ܣ�16λCRCУ�飬�ϵ� */ 
	SetCE;
	Delay(5);	/* ����оƬ�ֲ�Ҫ�� ����10us������ */
}

/*******************************************************************************
 * ����: nRF24L01_SendData
 * ����: �������ݺ���
 * �β�: data -> ָ��Ҫ��������ָ��
 * ����: ��
 * ˵��: 
 ******************************************************************************/
void nRF24L01_SendData(u8 *data)
{
	ClrCE;
	nRF24L01_Write_TxData(W_TX_PAYLOAD, data, TX_DATA_WITDH);	/* д��Ҫ���͵����� */
	SetCE;
	
	Delay(5);
}

/*******************************************************************************
 * ����: nRRF24L01_CheckACK
 * ����: ��ⷢ���Ƿ�ɹ��Լ������־λ����
 * �β�: 0--���ͳɹ���1--����ʧ��
 * ����: ��
 * ˵��: 
 ******************************************************************************/
u8 nRRF24L01_CheckACK()
{  
  	u8 sta;
	
	sta=nRF24L01_Read_Reg(R_REGISTER+STATUS);  	/* ���ͺ��ȡ״̬�Ĵ�����ֵ */
	UART1_SendByte(sta);
	if((sta&0x20)||(sta&0x10)) /* �Ƿ������������жϺ��ظ������ж� */
	{
	   nRF24L01_Write_Reg(W_REGISTER+STATUS,0xff);  /* ���TX_DS��MAX_RT�жϱ�־ */
	   ClrCSN;
	   nRF24L01_SPI_RW(FLUSH_TX);	/* �������FIFO */   
       SetCSN; 
	   return 0;
	}
	else
	   return 1;
}
