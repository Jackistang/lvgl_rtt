#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_port_fs.h"

#include <rtthread.h>

/* not needed, define LV_TICK_CUSTOM in lv_conf.h
#define LVGL_TICK_INTERVAL  5   

static void lvgl_tick(void *args)
{
    lv_tick_inc(LVGL_TICK_INTERVAL);
}

static void lvgl_tick_init(void)
{
    rt_timer_t lv_timer = rt_timer_create("lvgl.timer", lvgl_tick, RT_NULL, LVGL_TICK_INTERVAL, RT_IPC_FLAG_FIFO);
    if (lv_timer == RT_NULL)
        return ;
    
    rt_timer_start(lv_timer);
}
 */

static void lvgl_task(void *args)
{
    while (1)
    {
        rt_thread_mdelay(10);
        lv_task_handler();
    }
}

static int lvgl_task_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_thread_t tid = RT_NULL;

    /* littleGL demo gui thread */
    tid = rt_thread_create("lvgl.task", lvgl_task, RT_NULL, 1024 * 6, 5, 10);
    if (tid == RT_NULL)
    {
        return RT_ERROR;
    }
    ret = rt_thread_startup(tid);

    return ret;
}

static int lvgl_rtt_port_init(void)
{
    lv_init();
    lv_port_disp_init();

#if LV_USE_INDEV
    lv_port_indev_init();
#endif

#if LV_USE_FILESYSTEM
    lv_port_fs_init();
#endif

    lvgl_task_init();
    // lvgl_tick_init();   // not needed, define LV_TICK_CUSTOM in lv_conf.h

    return RT_EOK;
}
INIT_APP_EXPORT(lvgl_rtt_port_init);
