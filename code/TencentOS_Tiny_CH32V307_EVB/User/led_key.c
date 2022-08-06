#include "led_key.h"
#include "ch32v30x_gpio.h"


volatile uint16_t LED_1_Status = 0; 
volatile uint16_t LED_2_Status = 0; 
volatile uint16_t LED_3_Status = 0; 
volatile uint16_t LED_4_Status = 0; 

void led_key_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
     // for exti 
    EXTI_InitTypeDef EXTI_InitStructure = {0}; 
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_SetBits(GPIOE,GPIO_Pin_2);
    GPIO_SetBits(GPIOE,GPIO_Pin_3);
    GPIO_SetBits(GPIOE,GPIO_Pin_4);
    GPIO_SetBits(GPIOE,GPIO_Pin_5);
    /* key 1 2 3 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA0 -- EXTI_Line0 */    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  // rising edge
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // /* PA1 ----> EXTI_Line1 */    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  // rising edge
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // /* PA8 ----> EXTI_Line8  */    
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  // rising edge
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


// led-1 
void led_1_on()
{
    LED_1_Status = 1;
    GPIO_WriteBit(GPIOE, GPIO_Pin_2, LED_1_Status);
}

void led_1_off()
{
    LED_1_Status = 0;
    GPIO_WriteBit(GPIOE, GPIO_Pin_2, LED_1_Status);
}

void led_1_toggle()
{
    if(LED_1_Status == 0)
    {
        led_1_on();
    }
    else
    {
        led_1_off();
    }
}

// led-2
void led_2_on()
{
    LED_2_Status = 1;
    GPIO_WriteBit(GPIOE, GPIO_Pin_3, LED_2_Status);
}

void led_2_off()
{
    LED_2_Status = 0;
    GPIO_WriteBit(GPIOE, GPIO_Pin_3, LED_2_Status);
}

void led_2_toggle()
{
    if(LED_2_Status == 0)
    {
        led_2_on();
    }
    else
    {
        led_2_off();
    }
}
// led-3
void led_3_on()
{
    LED_3_Status = 1;
    GPIO_WriteBit(GPIOE, GPIO_Pin_4, LED_3_Status);
}

void led_3_off()
{
    LED_3_Status = 0;
    GPIO_WriteBit(GPIOE, GPIO_Pin_4, LED_3_Status);
}

void led_3_toggle()
{
    if(LED_3_Status == 0)
    {
        led_3_on();
    }
    else
    {
        led_3_off();
    }
}

// led-4
void led_4_on()
{
    LED_4_Status = 1;
    GPIO_WriteBit(GPIOE, GPIO_Pin_5, LED_4_Status);
}

void led_4_off()
{
    LED_4_Status = 0;
    GPIO_WriteBit(GPIOE, GPIO_Pin_5, LED_4_Status);
}

void led_4_toggle()
{
    if(LED_4_Status == 0)
    {
        led_4_on();
    }
    else
    {
        led_4_off();
    }
}

void led_all_off()
{
    led_1_off();
    led_2_off();
    led_3_off();
    led_4_off();
}

void led_all_on()
{
    led_1_on();
    led_2_on();
    led_3_on();
    led_4_on();
}