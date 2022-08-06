#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "debug.h"

#define USE_HORIZONTAL 0  //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����


#define LCD_W 240
#define LCD_H 240



//-----------------LCD�˿ڶ���---------------- 

#define LCD_SCLK_Clr() GPIO_WriteBit(GPIOE,GPIO_Pin_1,0)//SCL=SCLK
#define LCD_SCLK_Set() GPIO_WriteBit(GPIOE,GPIO_Pin_1,1)

#define LCD_MOSI_Clr() GPIO_WriteBit(GPIOD,GPIO_Pin_1,0)//SDA=MOSI
#define LCD_MOSI_Set() GPIO_WriteBit(GPIOD,GPIO_Pin_1,1)

#define LCD_RES_Clr()  GPIO_WriteBit(GPIOD,GPIO_Pin_3,0)//RES
#define LCD_RES_Set()  GPIO_WriteBit(GPIOD,GPIO_Pin_3,1)

#define LCD_DC_Clr()   GPIO_WriteBit(GPIOE,GPIO_Pin_0,0)//DC
#define LCD_DC_Set()   GPIO_WriteBit(GPIOE,GPIO_Pin_0,1)
 		     
//#define LCD_CS_Clr()   GPIO_WriteBit(GPIOA,GPIO_Pin_4)//CS
//#define LCD_CS_Set()   GPIO_WriteBit(GPIOA,GPIO_Pin_4)

#define LCD_BLK_Clr()  GPIO_WriteBit(GPIOD,GPIO_Pin_0,1)//BLK
#define LCD_BLK_Set()  GPIO_WriteBit(GPIOD,GPIO_Pin_0,0)




void LCD_GPIO_Init(void);//��ʼ��GPIO
void LCD_Writ_Bus(u8 dat);//ģ��SPIʱ��
void LCD_WR_DATA8(u8 dat);//д��һ���ֽ�
void LCD_WR_DATA(u16 dat);//д�������ֽ�
void LCD_WR_REG(u8 dat);//д��һ��ָ��
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//�������꺯��
void LCD_Init(void);//LCD��ʼ��
#endif




