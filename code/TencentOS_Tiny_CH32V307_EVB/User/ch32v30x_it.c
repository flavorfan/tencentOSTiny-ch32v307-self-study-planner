/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main Interrupt Service Routines.
*******************************************************************************/
#include "ch32v30x_it.h"
#include "tos_k.h"
#include "tos_at.h"

#include "led_key.h"

#include "pomodoro_clock.h"
#include "time.h"

extern at_agent_t esp8266_tf_agent;
extern at_agent_t esp8266_agent;

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

// add by flavor
void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
    printf("hardfault\r\n");
    printf("mepc   = %08x\r\n",__get_MEPC());
    printf("mcause = %08x\r\n",__get_MCAUSE());
    printf("mtval  = %08x\r\n",__get_MTVAL());
    while (1)
    {

    }
}

/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   This function handles USART2 global interrupt request.
 *
 * @return  none
 */
void USART2_IRQHandler(void)
{
  uint8_t data;
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {
      data= USART_ReceiveData(USART2);
      tos_at_uart_input_byte(&esp8266_tf_agent,data);
  }

}


/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   This function handles USART2 global interrupt request.
 *
 * @return  none
 */
void UART6_IRQHandler(void)
{
    uint8_t data;
  if(USART_GetITStatus(UART6, USART_IT_RXNE) != RESET)
  {
      data= USART_ReceiveData(UART6);
      tos_at_uart_input_byte(&esp8266_agent,data);

  }

}
/*********************************************************************
 * @fn      USART2_IRQHandler
 *
 * @brief   This function handles USART2 global interrupt request.
 *
 * @return  none
 */
void UART7_IRQHandler(void)
{
    uint8_t data;
  if(USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)
  {
      data= USART_ReceiveData(UART7);
      tos_at_uart_input_byte(&esp8266_tf_agent,data);
  }

}

// 1 = time

extern k_sem_t status_change;
extern k_chr_fifo_t status_fifo;


void mqtt_pub_evt(PomodoroPubType type)
{  
  tos_chr_fifo_push(&status_fifo, type);  // normal
  tos_sem_post(&status_change);
}


void EXTI0_IRQHandler(void)
{
  EXTI_ClearFlag(EXTI_Line0); // ????????????????????????
  led_1_toggle(); 
  // 
  // tos_chr_fifo_push(&status_fifo, 0);  // normal
  // tos_sem_post(&status_change);

  trigger_pomodoro();
}

void EXTI1_IRQHandler(void)
{
  EXTI_ClearFlag(EXTI_Line1); // ????????????????????????
  led_2_toggle();
  // 
  // tos_chr_fifo_push(&status_fifo, 1);  // 
  // tos_sem_post(&status_change);
  // add water 
  g_pomodoro_mng.w_cnt += 1;
  mqtt_pub_evt(PUB_TYPE_WATER);
  g_evt_tm = tos_systick_get();
}

void EXTI9_5_IRQHandler(void)
{
  EXTI_ClearFlag(EXTI_Line8); // ????????????????????????
  led_3_toggle();
  // 
  // tos_chr_fifo_push(&status_fifo, 2);
  // tos_sem_post(&status_change);
  // trigger_pomodoro();
  g_pomodoro_mng.todo_cnt += 1;
  mqtt_pub_evt(PUB_TYPE_TODO);
  g_evt_tm = tos_systick_get();
}
