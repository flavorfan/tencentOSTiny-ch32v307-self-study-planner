#ifndef __POMODORO_CLOCK_H__
#define __POMODORO_CLOCK_H__
#include "time.h"
#include "tos_k.h"

#define WATER_TOTAL 8

// struct tm* get_localtime(void);

void get_localtime(void);
void disp_date_time(u8 mode);
void disp_task(void);
void disp_pomodoro(void);

void daily_task_init(void);
void pomodoro_mng_init(void);

void trigger_pomodoro(void);
void pomodoro_counter_update(void);

typedef enum {
    PUB_TYPE_NTP = 0,
    PUB_TYPE_REP_ALL,
    PUB_TYPE_PO_START,
    PUB_TYPE_PO_END,
    PUB_TYPE_WATER,
    PUB_TYPE_TODO
} PomodoroPubType;

k_tick_t g_evt_tm;

struct sys_status_t
{
    u8 mqtt_ok;
    u8 ntp_ok;
    u8 cal_evt_ok;
};

struct sys_status_t g_sys_status;
struct pomodoro_mng
{
    // u8 status; // -> counter status
    u8 p_cnt;
    u8 b_cnt;
    u8 w_cnt;
    u8 todo_cnt;
    u8 todo_tal;    
};

// counter stuct for pomodoro
struct pomodoro_counter
{
    u8 pomodoro_state; // 0: idle, 1: work, 2: long break

    u32 pomodoro_cur;
    u32 pomodoro_total;
    u32 break_cur;
    u32 break_total;
    // u8 long_break_cur;
    // u8 long_break_total;
};

u8 b_pomodoro_mng_inited;

k_tick_t  g_last_time;
struct tm g_time_info;

struct pomodoro_mng g_pomodoro_mng;
char g_todo_cur[20];
char g_pomodoro_status[10];

struct pomodoro_counter g_pomodoro_counter;

u8 retry_wait;

#endif
