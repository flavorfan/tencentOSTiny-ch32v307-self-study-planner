#include "tos_k.h"

#define STK_SIZE_TASK_DEMO      512

k_stack_t stack_task_demo[STK_SIZE_TASK_DEMO];

k_task_t task_demo;


void entry_task_demo(void *arg)
{
    k_time_t ms;
    k_tick_t systick, after_systick;

    // 因为TOS_CFG_CPU_TICK_PER_SECOND为1000，也就是一秒钟会有1000个systick，因此1000个systick等于1000毫秒。
    systick = tos_millisec2tick(2000);
    printf("%d millisec equals to %lld ticks\n", 2000, systick);

    ms = tos_tick2millisec(1000);
    printf("%lld ticks equals to %d millisec\n", (k_tick_t)1000, ms);

    systick = tos_systick_get();
    printf("before sleep, systick is %lld\n", systick);

    tos_sleep_ms(2000);

    after_systick = tos_systick_get();
    printf("after sleep %d ms, systick is %lld\n", 2000, after_systick);

    tos_sleep_ms(2000);

    tos_systick_set(160000);
    tos_sleep_ms(2000);
    after_systick = tos_systick_get();
    printf("after systick set 16000 and sleep %d ms, systick is %lld\n", 2000, after_systick);

    printf("milliseconds sleep is about: %d\n", tos_tick2millisec(after_systick - systick));
}

void application_entry(void *arg)
{
    
    tos_task_create(&task_demo, "receiver_higher_prio", entry_task_demo, NULL,
                            4, stack_task_demo, STK_SIZE_TASK_DEMO, 0);
}
