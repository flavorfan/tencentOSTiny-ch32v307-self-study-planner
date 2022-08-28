#include "tos_k.h"
#include "esp8266_tencent_firmware.h"
#include "tencent_firmware_module_wrapper.h"

#include "cJSON.h"

#include "pomodoro_clock.h"
//#include "utils_json.h"

// #define PRODUCT_ID              "MOBA6ITP46"
// #define DEVICE_NAME             "dev001"
// #define DEVICE_KEY              "fuGPbKYNNP+kNj2gd/uXRQ=="


// for our product self-study partner
#define PRODUCT_ID              "QSKAOH3N71"
#define DEVICE_NAME             "dev001"
#define DEVICE_KEY              "rs62fMSZNSK4n2biq8Z59Q=="



#define REPORT_DATA_TEMPLATE      "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"status\\\":\\\"%s\\\"\\,\\\"p_cnt\\\":%d\\,\\\"b_cnt\\\":%d\\,\\\"w_cnt\\\":%d\\,\\\"todo_cnt\\\":%d\\,\\\"todo_tal\\\":%d\\,\\\"todo_cur\\\":\\\"%s\\\"}}"
#define SYS_MQTT_GET_RESOURCE_TIME     "{\\\"type\\\":\\\"get\\\"\\,\\\"resource\\\":[\\\"time\\\"]}"
// TODO： tm use timestamp format
#define REPORT_PO_START      "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"status\\\":\\\"%s\\\"\\,\\\"b_cnt\\\":%d\\,\\\"tm\\\":\\\"%lld\\\"}}"
#define REPORT_PO_END        "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"status\\\":\\\"%s\\\"\\,\\\"p_cnt\\\":%d\\,\\\"tm\\\":\\\"%lld\\\"}}"
#define REPORT_WATER         "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"w_cnt\\\":%d\\,\\\"tm\\\":\\\"%lld\\\"}}"
#define REPORT_TODO          "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"todo_cnt\\\":%d\\,\\\"tm\\\":\\\"%lld\\\"}}"

// #define REPORT_PO_START      "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"tm\\\":%lld\\,\\\"params\\\":{\\\"status\\\":\\\"%s\\\"\\,\\\"b_cnt\\\":%d}}"
// #define REPORT_PO_END        "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"tm\\\":%lld\\,\\\"params\\\":{\\\"status\\\":\\\"%s\\\"\\,\\\"p_cnt\\\":%d}}"
// #define REPORT_WATER         "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"tm\\\":%lld\\,\\\"params\\\":{\\\"w_cnt\\\":%d}}"
// #define REPORT_TODO          "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"tm\\\":%lld\\,\\\"params\\\":{\\\"todo_cnt\\\":%d}}"


#define MQTT_REPORT_TASK_STK_SIZE       4096
k_task_t mqtt_report_task;
__aligned(4) uint8_t mqtt_report_task_stk[MQTT_REPORT_TASK_STK_SIZE];

k_sem_t status_change;
k_chr_fifo_t status_fifo;
static char status_fifo_buf[128];



//extern int mqtt_report_task_init(void);

void default_message_handler(mqtt_message_t* msg)
{
    printf("callback:\r\n");
    printf("---------------------------------------------------------\r\n");
    printf("\ttopic:%s\r\n", msg->topic);
    printf("\tpayload:%s\r\n", msg->payload);
    printf("---------------------------------------------------------\r\n");
    // 
    g_sys_status.cal_evt_ok = 1;

}

 void ntp_message_handler(mqtt_message_t* msg)
 {
     //  {
     // 		 "type": "get",
     // 		 "time": 1621562342,
     // 		 "ntptime1": 1621562342773,
     // 		 "ntptime2": 1621562342773
     // }
     // from payload to result

     cJSON* cjson_root   = NULL;
     cJSON* cjson_time   = NULL;
     cJSON* cjson_ntptime1 = NULL;
     cJSON* cjson_ntptime2 = NULL;
    
     uint32_t time_get = 0;
     uint64_t ntptime1 = 0;
     uint64_t ntptime2 = 0;

     // printf("callback:\r\n");
     // printf("---------------------------------------------------------\r\n");
     printf("\ttopic:%s\r\n", msg->topic);
     printf("\tpayload:%s\r\n", msg->payload);
     // printf("---------------------------------------------------------\r\n");


     cjson_root = cJSON_Parse((char*)msg->payload + 1);
     if (cjson_root == NULL) {
         printf("report reply message parser fail\r\n");
         goto end;
     }

     // printf("json root:%s\r\n", cJSON_Print(cjson_root));
     // cJSON_Print(cjson_root);
     cjson_time = cJSON_GetObjectItem(cjson_root, "time");
     if (cjson_time == NULL) {
         printf("report reply message parser fail\r\n");
         goto end;
     }
     time_get = cjson_time->valueint;

     cjson_ntptime1 = cJSON_GetObjectItem(cjson_root, "ntptime1");
     if (cjson_ntptime1 == NULL) {
         printf("report reply message parser fail\r\n");
         goto end;
     }
     ntptime1 =  cjson_ntptime1->valuelonglong;

     cjson_ntptime2 = cJSON_GetObjectItem(cjson_root, "ntptime2");
     if (cjson_ntptime2 == NULL) {
         printf("report reply message parser fail\r\n");
         goto end;
     }
     ntptime2 = cjson_ntptime2->valuelonglong;

     printf("time_get:%d\r\n", time_get);
     printf("ntptime1:%lld \r\n", ntptime1);
     printf("ntptime2:%lld \r\n", ntptime2);

    tos_systick_set(ntptime2);
    g_last_time = ntptime2;

    // set ntp flag to true
    g_sys_status.ntp_ok = 1;
    retry_wait = 0;
 end:
     cJSON_Delete(cjson_root);
     cjson_root = NULL;
     return;

 }

 void data_message_handler(mqtt_message_t* msg)
 {
    // {
    //     "method": "event",
    //     "tm": 1660445974,
    //     "todo_cur": "english",
    //     "todo_tal": 3
    // }

     cJSON* cjson_root   = NULL;
     cJSON* cjson_todo_cur   = NULL;
     cJSON* cjson_todo_tal = NULL;
    //  cJSON* cjson_ntptime2 = NULL;
    

     // printf("callback:\r\n");
     // printf("---------------------------------------------------------\r\n");
     printf("\ttopic:%s\r\n", msg->topic);
     printf("\tpayload:%s\r\n", msg->payload);
     // printf("---------------------------------------------------------\r\n");


     cjson_root = cJSON_Parse((char*)msg->payload + 1);
     if (cjson_root == NULL) {
         printf("root parser fail\r\n");
         goto end;
     }


     cjson_todo_cur = cJSON_GetObjectItem(cjson_root, "todo_cur");
     if (cjson_todo_cur == NULL) {
         printf("todo_cur parser fail\r\n");
         goto end;
     }
    //  update the 
    memset(g_todo_cur, 0, sizeof(g_todo_cur));
    snprintf(g_todo_cur,20,cjson_todo_cur->valuestring);
    // time_get = cjson_todo_cur->valueint;

     cjson_todo_tal = cJSON_GetObjectItem(cjson_root, "todo_tal");
     if (cjson_todo_tal == NULL) {
         printf("todo_tal parser fail\r\n");
         goto end;
     }
    //  ntptime1 =  cjson_todo_tal->valuelonglong;
    g_pomodoro_mng.todo_tal = cjson_todo_tal->valueint;

    //  cjson_ntptime2 = cJSON_GetObjectItem(cjson_root, "ntptime2");
    //  if (cjson_ntptime2 == NULL) {
    //      printf("report reply message parser fail\r\n");
    //      goto end;
    //  }
    //  ntptime2 = cjson_ntptime2->valuelonglong;

     printf("todo_cur:%s\r\n", g_todo_cur);
     printf("todo_tal:%d \r\n", g_pomodoro_mng.todo_tal);
    //  printf("ntptime2:%lld \r\n", ntptime2);

    // tos_systick_set(ntptime2);
    // g_last_time = ntptime2;

    // set ntp flag to true
    // g_sys_status.ntp_ok = 1;
    // retry_wait = 0;
 end:
     cJSON_Delete(cjson_root);
     cjson_root = NULL;
     return;

 }

//  Todo:  void evt_message_handler(mqtt_message_t* msg)
//  sub 

char payload[256] = {0};
static char report_topic_name[TOPIC_NAME_MAX_SIZE] = {0};
static char report_reply_topic_name[TOPIC_NAME_MAX_SIZE] = {0};
// for ntp time sync
static char ntp_topic_name[TOPIC_NAME_MAX_SIZE] = {0};
static char ntp_replay_topic_name[TOPIC_NAME_MAX_SIZE] = {0};
static char dev_data_topic_name[TOPIC_NAME_MAX_SIZE] = {0};

void mqtt_report_task_entry(void *arg)
{
    int size = 0;
    mqtt_state_t state;
    char dev_status;
    k_err_t err;

    char *product_id = PRODUCT_ID;
    char *device_name = DEVICE_NAME;
    char *key = DEVICE_KEY;

    device_info_t dev_info;
    memset(&dev_info, 0, sizeof(device_info_t));

    strncpy(dev_info.product_id, product_id, PRODUCT_ID_MAX_SIZE);
    strncpy(dev_info.device_name, device_name, DEVICE_NAME_MAX_SIZE);
    strncpy(dev_info.device_serc, key, DEVICE_SERC_MAX_SIZE);
    tos_tf_module_info_set(&dev_info, TLS_MODE_PSK);

    mqtt_param_t init_params = DEFAULT_MQTT_PARAMS;
    if (tos_tf_module_mqtt_conn(init_params) != 0) {
        printf("module mqtt conn fail\n");
    } else {
        printf("module mqtt conn success\n");
    }

    if (tos_tf_module_mqtt_state_get(&state) != -1) {
        printf("MQTT: %s\n", state == MQTT_STATE_CONNECTED ? "CONNECTED" : "DISCONNECTED");
    }

    // sub -1 report_replay topic : 
    size = snprintf(report_reply_topic_name, TOPIC_NAME_MAX_SIZE, "$thing/down/property/%s/%s", product_id, device_name);

    if (size < 0 || size > sizeof(report_reply_topic_name) - 1) {
        printf("sub topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(report_reply_topic_name));
    }
    if (tos_tf_module_mqtt_sub(report_reply_topic_name, QOS0, default_message_handler) != 0) {
        printf("module mqtt sub fail\n");
    } else {
        printf("module mqtt sub success\n");
    }
    // sub -2 ntp topic 
    size = snprintf(ntp_replay_topic_name, TOPIC_NAME_MAX_SIZE, "$sys/operation/result/%s/%s", product_id, device_name);

    if (size < 0 || size > sizeof(ntp_replay_topic_name) - 1) {
        printf("sub topic 2 content length not enough! content size:%d  buf size:%d", size, (int)sizeof(ntp_replay_topic_name));
    }
    if (tos_tf_module_mqtt_sub(ntp_replay_topic_name, QOS0, ntp_message_handler) != 0) {
        printf("module mqtt sub 2 fail\n");
    } else {
        printf("module mqtt sub 2 success\n");
    }
    // sub -3 device data topic 
    size = snprintf(dev_data_topic_name, TOPIC_NAME_MAX_SIZE, "%s/%s/data", product_id, device_name);

    if (size < 0 || size > sizeof(dev_data_topic_name) - 1) {
        printf("sub topic 3 content length not enough! content size:%d  buf size:%d", size, (int)sizeof(dev_data_topic_name));
    }
    if (tos_tf_module_mqtt_sub(dev_data_topic_name, QOS0, data_message_handler) != 0) {
        printf("%s sub 3 fail\n",dev_data_topic_name );
    } else {
        printf("%s sub 3 success\n",dev_data_topic_name);
    }


    // pub -1 
    memset(report_topic_name, 0, sizeof(report_topic_name));
    size = snprintf(report_topic_name, TOPIC_NAME_MAX_SIZE, "$thing/up/property/%s/%s", product_id, device_name);

    if (size < 0 || size > sizeof(report_topic_name) - 1) {
        printf("pub topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(report_topic_name));
    }

    // pub -2 ntp topic
    memset(ntp_topic_name, 0, sizeof(ntp_topic_name));
    size = snprintf(ntp_topic_name, TOPIC_NAME_MAX_SIZE, "$sys/operation/%s/%s", product_id, device_name);

    if (size < 0 || size > sizeof(ntp_topic_name) - 1) {
        printf("pub topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(ntp_topic_name));
    }

    // setup mqtt flag to true
    g_sys_status.mqtt_ok = 1;

    while (1) {
        tos_sem_pend(&status_change, TOS_TIME_FOREVER);
        err = tos_chr_fifo_pop(&status_fifo, &dev_status);
        if (err != K_ERR_NONE) {
            printf("status_fifo pop fail, err is %d\r\n", err);
            continue;
        }

        // memset(payload, 0, sizeof(payload));
        // send diference signal   0: property report
        if (dev_status == PUB_TYPE_REP_ALL) {
            // snprintf(payload, sizeof(payload), REPORT_DATA_TEMPLATE_1, "work");
            snprintf(payload, sizeof(payload), REPORT_DATA_TEMPLATE, g_pomodoro_status,g_pomodoro_mng.p_cnt, g_pomodoro_mng.b_cnt, g_pomodoro_mng.w_cnt, g_pomodoro_mng.todo_cnt, g_pomodoro_mng.todo_tal, g_todo_cur);

            if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0) {
                printf("module mqtt pub fail\n");
                // break;
                continue;
            } else {
                printf("module mqtt pub success\n");
            }        
        } 
         // 1: ntp time sync
        else if (dev_status == PUB_TYPE_NTP) { 
            snprintf(payload, sizeof(payload), SYS_MQTT_GET_RESOURCE_TIME);
            if (tos_tf_module_mqtt_pub(ntp_topic_name, QOS0, payload) != 0) {
                printf("module mqtt pub fail\n");
                // break;
                continue;
            } else {
                printf("module mqtt pub success\n");
            }   
        } 
        // po_start
        else if (dev_status == PUB_TYPE_PO_START){
            snprintf(payload, sizeof(payload), REPORT_PO_START, g_pomodoro_status, g_pomodoro_mng.b_cnt,g_evt_tm);
            // snprintf(payload, sizeof(payload), REPORT_PO_START, g_evt_tm, g_pomodoro_status, g_pomodoro_mng.b_cnt );
            if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0) {
                printf("REPORT_PO_START pub fail\n");
                // break;
                continue;
            } else {
                printf("REPORT_PO_START pub success\n");
            }   
        }
        // po_end
        else if (dev_status == PUB_TYPE_PO_END){
            snprintf(payload, sizeof(payload), REPORT_PO_END, g_pomodoro_status, g_pomodoro_mng.p_cnt,g_evt_tm);
            // snprintf(payload, sizeof(payload), REPORT_PO_END, g_evt_tm, g_pomodoro_status, g_pomodoro_mng.p_cnt );
            if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0) {
                printf("PUB_TYPE_PO_END pub fail\n");
                // break;
                continue;
            } else {
                printf("PUB_TYPE_PO_END pub success\n");
            }   
        }
        // water
        else if (dev_status == PUB_TYPE_WATER){
            snprintf(payload, sizeof(payload), REPORT_WATER, g_pomodoro_mng.w_cnt, g_evt_tm);
            // snprintf(payload, sizeof(payload), REPORT_WATER, g_evt_tm, g_pomodoro_mng.w_cnt);
            if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0) {
                printf("PUB_TYPE_WATER pub fail\n");
                // break;
                continue;
            } else {
                printf("PUB_TYPE_WATER pub success\n");
            }   
        }
        // to do
        else if (dev_status == PUB_TYPE_TODO){
            snprintf(payload, sizeof(payload), REPORT_TODO, g_pomodoro_mng.todo_cnt, g_evt_tm);
            // snprintf(payload, sizeof(payload), REPORT_TODO,g_evt_tm, g_pomodoro_mng.todo_cnt );
            if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0) {
                printf("PUB_TYPE_TODO pub fail\n");
                // break;
                continue;
            } else {
                printf("PUB_TYPE_TODO pub success\n");
            }   
        }
        else {
            printf("device status unknown!");
            continue;
        }


    }
}

int mqtt_report_task_init(void)
{
   k_err_t err;

   err = tos_sem_create(&status_change, 0);
   if (err != K_ERR_NONE) {
       printf("status change sem create fail!\r\n");
       return -1;
   }

   err = tos_chr_fifo_create(&status_fifo, status_fifo_buf, sizeof(status_fifo_buf));
   if (err != K_ERR_NONE) {
      printf("status fifo create fail!\r\n");
      return -1;
   }

   err = tos_task_create(&mqtt_report_task, "mqtt_report_task", mqtt_report_task_entry, NULL, 4, mqtt_report_task_stk, MQTT_REPORT_TASK_STK_SIZE, 0);

   return err == K_ERR_NONE ? 0 : -1;
}
