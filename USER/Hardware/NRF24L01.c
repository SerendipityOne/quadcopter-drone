#include "NRF24L01.h"

/****************************************************************************************************/
#undef SUCCESS
#define SUCCESS 0
#undef FAILED
#define FAILED 1

#define MAX_TX 0x10  //达到最大发送次数中断
#define TX_OK  0x20  //TX发送完成中断
#define RX_OK  0x40  //接收到数据中断

/* 配对密码 */
const uint8_t TX_ADDRESS[] = {0xE1, 0xE2, 0xE3, 0xE4, 0xE5};  //本地地址
const uint8_t RX_ADDRESS[] = {0xE1, 0xE2, 0xE3, 0xE4, 0xE5};  //接收地址RX_ADDR_P0 == RX_ADDR

extern SPI_HandleTypeDef hspi2;
/****************************************************************************************************/
/* NRF24L01片选 */
#define Clr_NRF24L01_CE \
  HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET)
#define Set_NRF24L01_CE \
  HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_SET)

/* SPI片选 */
#define Clr_NRF24L01_CSN \
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET)
#define Set_NRF24L01_CSN \
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET)

/* IRQ */
#define READ_NRF24L01_IRQ \
  HAL_GPIO_ReadPin(NRF_IRQ_GPIO_Port, NRF_IRQ_Pin)
/****************************************************************************************************/

/**
 * @brief 初始化NRF24L01无线通信模块
 * 				该函数用于初始化NRF24L01模块，包括设置引脚状态和配置接收模式。
 * 				初始化过程中会持续检测模块状态，直到检测成功为止。
 */
void NRF24L01_Init(void) {
  Clr_NRF24L01_CE;
  Set_NRF24L01_CSN;

  do {
    RX_Mode();
  } while (NRF24L01_Check() == FAILED);
}

/**
 * @brief 检测NRF24L01模块是否在线
 * @param 无
 * @return 0: NRF24L01在位  1: NRF24L01不在位
 * 
 * 该函数通过向NRF24L01的发送地址寄存器写入特定数据，
 * 然后读取该数据进行比对来判断模块是否正常连接。
 */
uint8_t NRF24L01_Check(void) {
  uint8_t buf[5] = {0XA5, 0XA5, 0XA5, 0XA5, 0XA5};
  uint8_t buf1[5];
  uint8_t i;
  NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, buf, 5);  //写入5个字节的地址.
  NRF24L01_Read_Buf(TX_ADDR, buf1, 5);                  //读出写入的地址
  for (i = 0; i < 5; i++)
    if (buf1[i] != 0XA5) break;
  if (i != 5) return 1;  //NRF24L01不在位
  return 0;              //NRF24L01在位
}

/**
 * @brief   NRF24L01发送数据包函数
 * @param   txbuf - 指向要发送的数据缓冲区指针
 * @return  uint8_t - 发送结果状态
 *          SUCCESS: 发送成功
 *          MAX_TX:  达到最大重发次数
 *          FAILED:  其他原因发送失败
 * @note    该函数用于通过NRF24L01模块发送一个数据包
 */
uint8_t NRF24L01_TxPacket(uint8_t* txbuf) {
  uint8_t state;
  Clr_NRF24L01_CE;
  NRF24L01_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);  //写数据到TX BUF  32个字节
  Set_NRF24L01_CE;                                         //启动发送
  while (READ_NRF24L01_IRQ != 0);                          //等待发送完成
  state = NRF24L01_Read_Reg(STATUS);                       //读取状态寄存器的值
  NRF24L01_Write_Reg(SPI_WRITE_REG + STATUS, state);       //清除TX_DS或MAX_RT中断标志
  if (state & MAX_TX)                                      //达到最大重发次数
  {
    NRF24L01_Write_Reg(FLUSH_TX, 0xff);  //清除TX FIFO寄存器
    return MAX_TX;
  }
  if (state & TX_OK)  //发送完成
  {
    return SUCCESS;
  }
  return FAILED;  //其他原因发送失败
}

/**
 * @brief NRF24L01接收数据包函数
 * @param rxbuf 接收数据缓冲区指针，用于存储接收到的数据
 * @return uint8_t 操作结果，SUCCESS表示成功接收到数据，FAILED表示未接收到数据
 */
uint8_t NRF24L01_RxPacket(uint8_t* rxbuf) {
  uint8_t state;

  state = NRF24L01_Read_Reg(STATUS);                  //读取状态寄存器的值
  NRF24L01_Write_Reg(SPI_WRITE_REG + STATUS, state);  //清除TX_DS或MAX_RT中断标志
  if (state & RX_OK)                                  //接收到数据
  {
    NRF24L01_Read_Buf(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH);  //读取数据
    NRF24L01_Write_Reg(FLUSH_RX, 0xff);                     //清除RX FIFO寄存器

    return SUCCESS;
  }
  return FAILED;  //没收到任何数据
}

/**
 * @brief 设置NRF24L01模块进入接收模式
 * 
 * 该函数配置NRF24L01模块的各项寄存器参数，使其进入接收模式。
 * 配置包括设置接收地址、使能自动应答、设置通信频率、数据宽度等参数。
 * 
 * @param 无
 * @return 无
 */
void RX_Mode(void) {
  Clr_NRF24L01_CE;
  //写RX节点地址
  NRF24L01_Write_Buf(SPI_WRITE_REG + RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);

  //使能通道0的自动应答
  NRF24L01_Write_Reg(SPI_WRITE_REG + EN_AA, 0);
  //使能通道0的接收地址
  NRF24L01_Write_Reg(SPI_WRITE_REG + EN_RXADDR, 0x01);
  //设置RF通信频率
  NRF24L01_Write_Reg(SPI_WRITE_REG + RF_CH, 1);
  //选择通道0的有效数据宽度
  NRF24L01_Write_Reg(SPI_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);
  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
  NRF24L01_Write_Reg(SPI_WRITE_REG + RF_SETUP, 0x07);
  //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,PRIM_RX接收模式
  NRF24L01_Write_Reg(SPI_WRITE_REG + NCONFIG, 0x0f);
  //CE为高,进入接收模式
  Set_NRF24L01_CE;
}

// /**
//  * @brief 进入NRF24L01发送模式并初始化相关寄存器配置
//  *
//  * 该函数用于将NRF24L01模块配置为发送模式，包括设置发送地址、接收地址（用于ACK）、
//  * 自动应答、重发机制、射频参数等。配置完成后拉高CE引脚以启动发送。
//  *
//  * @param 无
//  * @return 无
//  */
// void TX_Mode(void) {
//   Clr_NRF24L01_CE;
//   //写TX节点地址
//   NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, (uint8_t*)TX_ADDRESS, TX_ADR_WIDTH);
//   //设置TX节点地址,主要为了使能ACK
//   NRF24L01_Write_Buf(SPI_WRITE_REG + RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);

//   //使能通道0的自动应答
//   NRF24L01_Write_Reg(SPI_WRITE_REG + EN_AA, 0x01);
//   //使能通道0的接收地址
//   NRF24L01_Write_Reg(SPI_WRITE_REG + EN_RXADDR, 0x01);
//   //设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
//   NRF24L01_Write_Reg(SPI_WRITE_REG + SETUP_RETR, 0x1a);
//   //设置RF通道为40
//   NRF24L01_Write_Reg(SPI_WRITE_REG + RF_CH, 40);
//   //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
//   NRF24L01_Write_Reg(SPI_WRITE_REG + RF_SETUP, 0x0f);  //0x27  250K   0x07 1M
//                                                        //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,PRIM_RX发送模式,开启所有中断
//   NRF24L01_Write_Reg(SPI_WRITE_REG + NCONFIG, 0x0e);
//   // CE为高,10us后启动发送
//   Set_NRF24L01_CE;
// }

/****************************************************************************************************/
/* 以下是NRF24L01驱动函数                       																										  */
/****************************************************************************************************/

/**
 * @brief 通过SPI接口发送一个字节数据到NRF模块，并返回接收到的数据
 * @param b 要发送的字节数据
 * @return 接收到的字节数据
 */
uint8_t NRF_SPI_SendByte(uint8_t b) {
  uint8_t rxdata;
  HAL_SPI_TransmitReceive(&hspi2, &b, &rxdata, 1, 0x10);
  return rxdata;
}

/**
 * @brief 向NRF24L01写入寄存器数据
 * @param regaddr 寄存器地址
 * @param data 要写入的数据
 * @return 返回写入操作的状态值
 */
uint8_t NRF24L01_Write_Reg(uint8_t regaddr, uint8_t data) {
  uint8_t status;
  Clr_NRF24L01_CSN;                    //使能SPI传输
  status = NRF_SPI_SendByte(regaddr);  //发送寄存器号
  NRF_SPI_SendByte(data);              //写入寄存器的值
  Set_NRF24L01_CSN;                    //禁止SPI传输
  return (status);                     //返回状态值
}

/**
 * @brief 读取NRF24L01寄存器值
 * @param regaddr 寄存器地址
 * @return 读取到的寄存器值
 */
uint8_t NRF24L01_Read_Reg(uint8_t regaddr) {
  uint8_t reg_val;
  Clr_NRF24L01_CSN;                  //使能SPI传输
  NRF_SPI_SendByte(regaddr);         //发送寄存器号
  reg_val = NRF_SPI_SendByte(0XFF);  //读取寄存器内容
  Set_NRF24L01_CSN;                  //禁止SPI传输
  return (reg_val);                  //返回状态值
}

/**
 * @brief 从NRF24L01指定寄存器读取数据到缓冲区
 * @param regaddr 要读取的寄存器地址
 * @param pBuf 数据存储缓冲区指针
 * @param datalen 要读取的数据长度
 * @return 返回读取操作的状态值
 */
uint8_t NRF24L01_Read_Buf(uint8_t regaddr, uint8_t* pBuf, uint8_t datalen) {
  uint8_t status;
  Clr_NRF24L01_CSN;                    //使能SPI传输
  status = NRF_SPI_SendByte(regaddr);  //发送寄存器值(位置),并读取状态值
  HAL_SPI_Receive(&hspi2, pBuf, datalen, 0x10);
  Set_NRF24L01_CSN;  //关闭SPI传输
  return status;     //返回读到的状态值
}

/**
 * @brief 向NRF24L01写入缓冲区数据
 * @param regaddr 寄存器地址
 * @param pBuf 指向要写入数据的缓冲区指针
 * @param datalen 要写入的数据长度
 * @return 返回写入操作后的状态值
 */
uint8_t NRF24L01_Write_Buf(uint8_t regaddr, uint8_t* pBuf, uint8_t datalen) {
  uint8_t status;
  Clr_NRF24L01_CSN;                    //使能SPI传输
  status = NRF_SPI_SendByte(regaddr);  //发送寄存器值(位置),并读取状态值
  HAL_SPI_Transmit(&hspi2, pBuf, datalen, 0x10);
  Set_NRF24L01_CSN;  //关闭SPI传输
  return status;     //返回读到的状态值
}
