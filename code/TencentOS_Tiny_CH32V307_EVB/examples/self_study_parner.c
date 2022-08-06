#include "debug.h"
#include "tos_k.h"
#include "esp8266_tencent_firmware.h"
#include "tencent_firmware_module_wrapper.h"

// task

void application_entry(void *arg)
{

    // wifi init
    int ret;

    ret = esp8266_tencent_firmware_sal_init(HAL_UART_PORT_2);
    if (ret < 0) {
        printf("esp8266 tencent firmware sal init fail, ret is %d\r\n", ret);
        return;
    }

    printf("connect wifi...\r\n");
    //LCD_ShowString(30,140+16+16+16,"connect wifi...",WHITE,BLACK,16,0);

    while (esp8266_tencent_firmware_join_ap("xxxxx", "xxxxxx") != 0) {
        printf("connect wifi... fail!\r\n");
        // LCD_ShowString(30,140+16+16+16,"connect wifi fail",WHITE,BLACK,16,0);
    }

    printf("connect wifi... ok!\r\n");
    //LCD_ShowString(30,140+16+16+16,"connect wifi ok",WHITE,BLACK,16,0);

    // task 1 -- mqtt_report_task_init
    ret = mqtt_report_task_init();
    if (ret < 0) {
        printf("mqtt report task create fail!\r\n");
        return;
    }

    // task 2 -
    // ret = camera_task_init();
    // if (ret < 0) {
    //     printf("camera_task_init create fail!\r\n");
    //     return;
    // }

}