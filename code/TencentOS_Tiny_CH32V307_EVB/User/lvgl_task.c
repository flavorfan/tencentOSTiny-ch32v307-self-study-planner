#include "tos_k.h"

// task
// lvgl port -step 1:  include the lvgl
#include "lvgl/lvgl.h"
#include "lvgl/examples/porting/lv_port_disp.h"
#include "lvgl/src/hal/lv_hal_tick.h"
#define LV_ATTRIBUTE_TICK_INC

#include "lcd.h"
#include "lcd_init.h"
// lvgl port -step 2:  config function

// tos task setting 
#define LVGL_TASK_STK_SIZE       512
k_task_t lvgl_task;
__aligned(4) uint8_t lvgl_task_stk[LVGL_TASK_STK_SIZE];
// tos semaphore 

// lvgl app code


static lv_obj_t * meter;

static void set_value(void * indic, int32_t v)
{
    lv_meter_set_indicator_end_value(meter, indic, v);
}

void lv_example_meter_xxxx(void)
{
    meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 200, 200);

    /*Remove the circle from the middle*/
    lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 11, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 1, 2, 30, lv_color_hex3(0xeee), 10);
    lv_meter_set_scale_range(meter, scale, 0, 100, 270, 90);

    /*Add a three arc indicator*/
    lv_meter_indicator_t * indic1 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_indicator_t * indic2 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_GREEN), -10);
    lv_meter_indicator_t * indic3 = lv_meter_add_arc(meter, scale, 10, lv_palette_main(LV_PALETTE_BLUE), -20);

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

    lv_anim_set_time(&a, 2000);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_var(&a, indic1);
    lv_anim_start(&a);

    lv_anim_set_time(&a, 1000);
    lv_anim_set_playback_time(&a, 1000);
    lv_anim_set_var(&a, indic2);
    lv_anim_start(&a);

    lv_anim_set_time(&a, 1000);
    lv_anim_set_playback_time(&a, 2000);
    lv_anim_set_var(&a, indic3);
    lv_anim_start(&a);
}

void periodic_timer_cb(void *arg)
{
    lv_tick_inc(10);
    lv_timer_handler();
}



void lvgl_task_entry(void *arg)
{
    k_timer_t periodic_tmr;    
    // timer_handler 10ms?
    tos_timer_create(&periodic_tmr, 2000, 10, periodic_timer_cb, K_NULL, TOS_OPT_TIMER_PERIODIC);
    tos_timer_start(&periodic_tmr);

}



int lvgl_task_init(void)
{
    
    k_err_t err;

    lv_init();
    lv_port_disp_init();
    lv_example_meter_xxxx();

    err = tos_task_create(&lvgl_task, "lvgl", lvgl_task_entry, NULL,
                            5, lvgl_task_stk, LVGL_TASK_STK_SIZE, 0);

    
    return err == K_ERR_NONE ? 0 : -1;
}