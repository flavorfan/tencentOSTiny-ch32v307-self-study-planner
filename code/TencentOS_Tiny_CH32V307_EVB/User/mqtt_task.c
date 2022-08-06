#include "tos_k.h"
#include "esp8266_tencent_firmware.h"
#include "tencent_firmware_module_wrapper.h"

#include "cJSON.h"
//#include "utils_json.h"


// for our product self-study partner
#define PRODUCT_ID              "xxxxxxx"
#define DEVICE_NAME             "xxxxx"
#define DEVICE_KEY              "xxxxxxxxxx


#define REPORT_DATA_TEMPLATE    "{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"status\\\":\\\"%s\\\"}}"
#define SYS_MQTT_GET_RESOURCE_TIME     "{\\\"type\\\":\\\"get\\\"\\,\\\"resource\\\":[\\\"time\\\"]}"

#define MQTT_REPORT_TASK_STK_SIZE       4096
k_task_t mqtt_report_task;
__aligned(4) uint8_t mqtt_report_task_stk[MQTT_REPORT_TASK_STK_SIZE];

k_sem_t status_change;
k_chr_fifo_t status_fifo;
static char status_fifo_buf[128];

void default_message_handler(mqtt_message_t* msg)
{
    printf("callback:\r\n");
    printf("---------------------------------------------------------\r\n");
    printf("\ttopic:%s\r\n", msg->topic);
    printf("\tpayload:%s\r\n", msg->payload);
    printf("---------------------------------------------------------\r\n");
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

     k_tick_t local_time_tick = 0;



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


     // 
//     time_get = tos_systick_get();
//     print("systick %d\r\n", time_get);

 //
 end:
     cJSON_Delete(cjson_root);
     cjson_root = NULL;
     return;

 }

char payload[256] = {0};
static char report_topic_name[TOPIC_NAME_MAX_SIZE] = {0};
static char report_reply_topic_name[TOPIC_NAME_MAX_SIZE] = {0};
// for ntp time sync
static char ntp_topic_name[TOPIC_NAME_MAX_SIZE] = {0};
static char ntp_replay_topic_name[TOPIC_NAME_MAX_SIZE] = {0};

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


    while (1) {
        tos_sem_pend(&status_change, TOS_TIME_FOREVER);
        err = tos_chr_fifo_pop(&status_fifo, &dev_status);
        if (err != K_ERR_NONE) {
            printf("status_fifo pop fail, err is %d\r\n", err);
            continue;
        }

        // send diference signal 
        if (dev_status == 0) {
            snprintf(payload, sizeof(payload), REPORT_DATA_TEMPLATE, "start");    

            if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0) {
                printf("module mqtt pub fail\n");
                break;
            } else {
                printf("module mqtt pub success\n");
            }        
        } 
        else if (dev_status == 1) {
            snprintf(payload, sizeof(payload), REPORT_DATA_TEMPLATE, "stop");      
            
            if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0) {
                printf("module mqtt pub fail\n");
                break;
            } else {
                printf("module mqtt pub success\n");
            }         
        } 
        else if (dev_status == 2) {
            // snprintf(payload, sizeof(payload), REPORT_DATA_TEMPLATE, "time");

            // payload     = SYS_MQTT_GET_RESOURCE_TIME;
            snprintf(payload, sizeof(payload), SYS_MQTT_GET_RESOURCE_TIME);
            if (tos_tf_module_mqtt_pub(ntp_topic_name, QOS0, payload) != 0) {
                printf("module mqtt pub fail\n");
                // break;
                continue;
            } else {
                printf("module mqtt pub success\n");
            }   

        } else {
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
