#include "debug.h"
#include "tos_k.h"
#include "esp8266_tencent_firmware.h"
#include "tencent_firmware_module_wrapper.h"

// timer -> pomodoro clock  
#include "pomodoro_clock.h"
#include "lcd.h"

extern int mqtt_report_task_init(void);
extern int lvgl_task_init(void);

k_timer_t periodic_tmr;


void periodic_timer_cb(void *arg)
{
    //printf("this is periodic timer callback, current systick: %lld\n", tos_systick_get());
    pomodoro_counter_update();
    
}


void application_entry(void *arg)
{

    // wifi init
    int ret;

    //--------- init 
    pomodoro_mng_init();

    //--------- setup
    ret = esp8266_tencent_firmware_sal_init(HAL_UART_PORT_2);
    if (ret < 0) {
        printf("esp8266 tencent firmware sal init fail, ret is %d\r\n", ret);
        return;
    }

    printf("connect wifi...\r\n");
    LCD_ShowString(0,20,"connect wifi...",WHITE,BLACK,16,0);

    while (esp8266_tencent_firmware_join_ap("flavor", "12345687") != 0) {
        printf("connect wifi... fail!\r\n");
        LCD_ShowString(0,20,"connect wifi fail",WHITE,BLACK,16,0);
    }

    printf("connect wifi... ok!\r\n");
    LCD_ShowString(0,20,"connect wifi ok",WHITE,BLACK,16,0);

    // task 1 -- mqtt_report_task_init
    ret = mqtt_report_task_init();
    if (ret < 0) {
        printf("mqtt report task create fail!\r\n");
        return;
    }

    // timer 
    tos_timer_create(&periodic_tmr, 2000, 1000, periodic_timer_cb, K_NULL, TOS_OPT_TIMER_PERIODIC);
    tos_timer_start(&periodic_tmr);

//    task 2 - lvgl
//    ret = lvgl_task_init();
//    if (ret < 0) {
//        printf("lvgl task create fail!\r\n");
//        return;
//    }

}
