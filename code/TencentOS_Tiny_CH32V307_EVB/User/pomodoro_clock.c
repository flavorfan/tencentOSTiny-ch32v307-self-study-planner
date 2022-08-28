#include "pomodoro_clock.h"
#include "time.h"
#include "lcd.h"

// bit enable of display type

struct tm g_time_info = {0};
k_tick_t  g_last_time = 0;
u8 b_pomodoro_mng_inited = 0;

char g_todo_cur[20] = {0};
char g_pomodoro_status[10] = {0};

k_tick_t g_evt_tm = 0;

u8 retry_wait = 0;


extern k_sem_t status_change;
extern k_chr_fifo_t status_fifo;


void pomodoro_counter_init(void)
{
    g_pomodoro_counter.pomodoro_state = 0;  // 0: idle, 1: work, 2: long break
    snprintf(g_pomodoro_status, 10, "idle");
    g_pomodoro_counter.pomodoro_cur = 0;
    g_pomodoro_counter.pomodoro_total = 60*25;
    g_pomodoro_counter.break_cur = 0;
    g_pomodoro_counter.break_total = 60 * 15;
}


void pomodoro_mng_init(void)
{
    //g_sys_status init
    g_sys_status.mqtt_ok = 0;
    g_sys_status.ntp_ok = 0;
    g_sys_status.cal_evt_ok = 0;    

    // g_pomodoro_mng init
    g_pomodoro_mng.p_cnt = 0;
    g_pomodoro_mng.b_cnt = 0;
    g_pomodoro_mng.w_cnt = 0;
    g_pomodoro_mng.todo_cnt = 0;
    g_pomodoro_mng.todo_tal = 0;
    // g_pomodoro_mng.todo_cur = "English";   
    // memset(g_todo_cur, 0, sizeof(g_todo_cur));
    snprintf(g_todo_cur, 20, "English");
    snprintf(g_pomodoro_status, 10, "idle");
    b_pomodoro_mng_inited = 1;

    pomodoro_counter_init();
}

void trigger_pomodoro(void)
{
    g_evt_tm = tos_systick_get();

    if (g_pomodoro_counter.pomodoro_state == 0)
    {
        g_pomodoro_counter.pomodoro_state = 1; 
        snprintf(g_pomodoro_status, 10, "work");
        g_pomodoro_counter.pomodoro_cur = 60*25; 
        g_pomodoro_counter.pomodoro_total = 60*25;
        // trigger start
        tos_chr_fifo_push(&status_fifo, PUB_TYPE_PO_START);  
        tos_sem_post(&status_change);
    }
    else if (g_pomodoro_counter.pomodoro_state == 1)
    {
        g_pomodoro_counter.pomodoro_state = 2;
        snprintf(g_pomodoro_status, 10, "break");
        g_pomodoro_counter.break_cur =60 * 15;
        g_pomodoro_counter.break_total = 60 * 15;

        // trigger
        g_pomodoro_mng.b_cnt += 1;
        tos_chr_fifo_push(&status_fifo, PUB_TYPE_PO_END);  
        tos_sem_post(&status_change);
    }
    else if (g_pomodoro_counter.pomodoro_state == 2)
    {
        g_pomodoro_counter.pomodoro_state = 1;
        snprintf(g_pomodoro_status, 10, "work"); 
        g_pomodoro_counter.pomodoro_cur = 60*25; 
        g_pomodoro_counter.pomodoro_total = 60*25;

        // trigger
        g_pomodoro_mng.p_cnt += 1;
        tos_chr_fifo_push(&status_fifo, PUB_TYPE_PO_START);  
        tos_sem_post(&status_change);
    }
   
}

void pomodoro_counter_update(void)
{
    // only once
    if (g_sys_status.mqtt_ok == 1 && g_sys_status.ntp_ok == 0 )
    {
        if (retry_wait == 0){
            tos_chr_fifo_push(&status_fifo, PUB_TYPE_NTP);  // for ntp  100
            tos_sem_post(&status_change);
            retry_wait = 10;
           
        }
        retry_wait--;
    }
    else if (g_sys_status.ntp_ok && g_sys_status.cal_evt_ok == 0)
    {
        if (retry_wait == 0){
            tos_chr_fifo_push(&status_fifo, PUB_TYPE_REP_ALL);  // for first report 110
            tos_sem_post(&status_change);
            retry_wait = 10;
        }
        retry_wait--;
         
        disp_date_time(1);  // display all
        disp_pomodoro();
    }
    // else {
    //     // 111 : reflash 
    //     disp_task();
    // }
    

    // counter state udpate : 0 idle, 1 work, 2 break
    // if (g_pomodoro_counter.pomodoro_state == 0)
    // {
    //     return;
    // }

    if (g_pomodoro_counter.pomodoro_state == 1)
    {        
        if (g_pomodoro_counter.pomodoro_cur > 0)
        {
            g_pomodoro_counter.pomodoro_cur--;
        }        
    }
    else if (g_pomodoro_counter.pomodoro_state == 2)
    {
        if (g_pomodoro_counter.break_cur > 0)
        {
            g_pomodoro_counter.break_cur--;
        }
    }
    disp_pomodoro();
    disp_date_time(0);  // display all
    disp_task();
}


void get_localtime()
{
    k_tick_t now;
    struct tm *time_info_get;

    //shanghai +8: + 28800
    now = tos_systick_get() / 1000 + 28800 ;
    // printf("now: %lld sec \r\n", now);
    time_info_get = localtime(&now);
    // printf("%d-%d-%d %d:%d:%d\r\n", time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday, time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
    g_time_info = *time_info_get;
    // return time_info;
}

// mode: 1 update date also
void disp_date_time(u8 mode)
{
    // struct tm *time_info;

    char lcd_buf[30] = {0};
    // time_info = get_localtime();
    get_localtime();
        
    if(mode)
    {
        sprintf(lcd_buf, "%4d-%02d-%02d", g_time_info.tm_year + 1900, g_time_info.tm_mon + 1, g_time_info.tm_mday);
        LCD_ShowString(0,80,lcd_buf, WHITE, BLACK, 32, 0);
    }    
    sprintf(lcd_buf, "%02d:%02d:%02d", g_time_info.tm_hour, g_time_info.tm_min, g_time_info.tm_sec);
    LCD_ShowString(0,110,lcd_buf, WHITE, BLACK, 32, 0); 
}

void disp_task()
{
    char lcd_buf[30] = {0};

    if (!b_pomodoro_mng_inited){
        pomodoro_mng_init();        
    }

    sprintf(lcd_buf, "P:%d  W:%d|%d  T:%d|%d", g_pomodoro_mng.p_cnt, g_pomodoro_mng.w_cnt, WATER_TOTAL, g_pomodoro_mng.todo_cnt, g_pomodoro_mng.todo_tal);
    LCD_ShowString(0,150,lcd_buf, WHITE, BLACK, 16, 0); 

    sprintf(lcd_buf, "> %10s", g_todo_cur);
    LCD_ShowString(0,180,lcd_buf, WHITE, BLACK, 32, 0); 

}
void disp_pomodoro()
{
    char lcd_buf[10] = {0};

    if (g_pomodoro_counter.pomodoro_state == 1)
    {
        sprintf(lcd_buf, "Work  %02d:%02d", g_pomodoro_counter.pomodoro_cur/60, g_pomodoro_counter.pomodoro_cur%60);
    }
    else if (g_pomodoro_counter.pomodoro_state == 2)
    {
        sprintf(lcd_buf, "Break %02d:%02d", g_pomodoro_counter.break_cur/60, g_pomodoro_counter.break_cur%60);
    }
    else
    {
        sprintf(lcd_buf, "Idle 00:00");
    }    
    LCD_ShowString(0,20,lcd_buf, RED, BLACK, 32, 0); 
}
