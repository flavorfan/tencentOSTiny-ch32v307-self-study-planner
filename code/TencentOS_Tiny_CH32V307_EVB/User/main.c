/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*******************************************************************************/
#include "debug.h"
#include "tos_k.h"
#include "sdio.h"
#include "lcd_init.h"
#include "lcd.h"
#include "pic.h"
#include "spi_flash.h"

#include "led_key.h"



uint8_t get_key(void)
{
    uint16_t value=0;
    if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) & GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)\
         & GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)) == 0 )
    {
        Delay_Ms(15);
        if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) & GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)\
             & GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)) == 0 )
        {
           value= GPIO_ReadInputData(GPIOA);
           if((value&0x1)==0) return 1;
           else if((value&0x2)==0)return 2;
           else if((value&0x100)==0) return 3;
           else return 0;
        }
    }
   return 0;
}

void lcd_test()
{
    u8 i,j;
    float t=0;

    LCD_ShowString(0,0,"Welcome TencentOS",WHITE,BLACK,32,0);
    LCD_ShowString(0,40,"LCD_W:",WHITE,BLACK,16,0);
    LCD_ShowIntNum(48,40,LCD_W,3,WHITE,BLACK,16);
    LCD_ShowString(80,40,"LCD_H:",WHITE,BLACK,16,0);
    LCD_ShowIntNum(128,40,LCD_H,3,WHITE,BLACK,16);
    LCD_ShowString(80,40,"LCD_H:",WHITE,BLACK,16,0);
    LCD_ShowString(0,70,"Increaseing Nun:",WHITE,BLACK,16,0);
    LCD_ShowFloatNum1(128,70,t,4,WHITE,BLACK,16);
    t+=0.11;
    for(j=0;j<3;j++)
    {
        for(i=0;i<6;i++)
        {
            LCD_ShowPicture(40*i,120+j*40,40,40,gImage_1);
        }
    }
    printf("lcd test finish\r\n");
}

/*********************************************************************
 * @fn      show_sdcard_info
 *
 * @brief   SD Card information.
 *
 * @return  none
 */
void show_sdcard_info(void)
{
    switch(SDCardInfo.CardType)
    {
        case SDIO_STD_CAPACITY_SD_CARD_V1_1:printf("Card Type:SDSC V1.1\r\n");break;
        case SDIO_STD_CAPACITY_SD_CARD_V2_0:printf("Card Type:SDSC V2.0\r\n");break;
        case SDIO_HIGH_CAPACITY_SD_CARD:printf("Card Type:SDHC V2.0\r\n");break;
        case SDIO_MULTIMEDIA_CARD:printf("Card Type:MMC Card\r\n");break;
    }
    printf("Card ManufacturerID:%d\r\n",SDCardInfo.SD_cid.ManufacturerID);
    printf("Card RCA:%d\r\n",SDCardInfo.RCA);
    printf("Card Capacity:%d MB\r\n",(u32)(SDCardInfo.CardCapacity>>20));
    printf("Card BlockSize:%d\r\n\r\n",SDCardInfo.CardBlockSize);
}
const u8 TEXT_Buf[]={"CH32V307 SPI FLASH W25Qxx"};
void spi_flash_test(void)
{
    u8 datap[SIZE];
    u16 Flash_Model;
//    SPI_Flash_Init();
    Delay_Ms(50);

    Flash_Model = SPI_Flash_ReadID();

    switch(Flash_Model)
    {
       case W25Q80:
            printf("W25Q80 OK!\r\n");
            break;

       case W25Q16:
            printf("W25Q16 OK!\r\n");
            break;

       case W25Q32:
            printf("W25Q32 OK!\r\n");
            break;

       case W25Q64:
            printf("W25Q64 OK!\r\n");
            break;

       case W25Q128:
            printf("W25Q128 OK!\r\n");
            break;

       default:
            printf("Fail!\r\n");
            break;
    }
    printf("Start Read W25Qxx before erase....\r\n");
    SPI_Flash_Read(datap,0x0,SIZE);
    for(int16_t i=0;i<SIZE;i++)
    {
        printf("%x ", datap[i] );
    }
    printf("\r\n" );

    printf("Start Erase W25Qxx....\r\n");
    SPI_Flash_Erase_Sector(0);

    Delay_Ms(500);

    printf("Start Read W25Qxx....\r\n");
    SPI_Flash_Read(datap,0x0,SIZE);
    for(int16_t i=0;i<SIZE;i++)
    {
        printf("%x ", datap[i] );
    }
    printf("\r\n" );

    Delay_Ms(500);

    printf("Start Write W25Qxx....\r\n");
    SPI_Flash_Write((u8*)TEXT_Buf,0,SIZE);

    Delay_Ms(500);

    SPI_Flash_Read(datap,0x0,SIZE);
    printf("%s\r\n", datap );

    if(memcmp(TEXT_Buf, datap, SIZE))
    {
        LCD_ShowString(16,16+16+16,"SPI Flash: Fail",WHITE,BLACK,16,0);
    }
    else
    {
        LCD_ShowString(16,16+16+16,"SPI Flash: OK",WHITE,BLACK,16,0);
    }
}

u8 buf[512];
u8 Readbuf[512];

/* sdcard test */
void SD_Card_test(void)
{
    u32 i;
    u32 Sector_Nums;

    while(SD_Init())
    {
        printf("SD Card Error!\r\n");
        delay_ms(1000);
    }

    show_sdcard_info();

    printf("SD Card OK\r\n");
    Sector_Nums = ((u32)(SDCardInfo.CardCapacity>>20))/2;
    printf("Sector_Nums:%d\r\n", Sector_Nums);

    for(i=0; i<512; i++){
        buf[i] = i;
    }

    for(i=0; i<Sector_Nums/2; i++)
    {
        if(SD_WriteDisk(buf,i,1)) {printf("Wr %d sector fail\n", i);}
        if(SD_ReadDisk(Readbuf,i,1)){ printf("Rd %d sector fail\n", i);}

        if(memcmp(buf, Readbuf, 512))
        {
            printf(" %d sector Verify fail\n", i);
            LCD_ShowString(16,16+16+16+16,"SD Sector:FAIL",WHITE,BLACK,16,0);
            break;
        }
    }
    if(i==Sector_Nums/2)
    {
        LCD_ShowString(16,16+16+16+16+16,"SD: OK",WHITE,BLACK,16,0);
        printf("SD OK\r\n");
    }
    else
    {
        LCD_ShowString(16,16+16+16+16+16,"SD ALL: FAIL",WHITE,BLACK,16,0);
    }
}

#define APPLICATION_TASK_STK_SIZE       4096
k_task_t application_task;
__aligned(4) uint8_t application_task_stk[APPLICATION_TASK_STK_SIZE];

extern void application_entry(void *arg);

volatile uint8_t key=0;
/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Return         : None
*******************************************************************************/
int main(void)
{
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n",SystemCoreClock);
	led_key_init();
    LCD_Init();
    SPI_Flash_Init();
    printf("Welcome to TencentOS tiny(%s)\r\n", TOS_VERSION);
    LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
    LCD_ShowString(0,0,"Welcome to TencentOS",WHITE,BLACK,16,0);

    // init 
    led_all_off();    

    tos_knl_init();
    tos_task_create(&application_task, "application_task", application_entry, NULL, 4, application_task_stk, APPLICATION_TASK_STK_SIZE, 0);
    
    tos_knl_start();

    printf("should not run at here!\r\n");
    while(!key)
    {
        key=get_key();
    }

    switch(key)
    {
        case 1:
            lcd_test();
            break;
        case 2:
            LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
            spi_flash_test();
            SD_Card_test();
            break;
        case 3:
            LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
            break;
        default:
            break;
    }
	while(1);
}
