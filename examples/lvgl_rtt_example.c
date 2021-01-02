#include "lv_test_theme.h"

#include <rtthread.h>

static void lvgl_demo_run(void *p)
{
    lv_demo_widgets();
}

static int lvgl_rtt_demo_init(void)
{
    rt_thread_t thread = RT_NULL;

    /* littleGL demo gui thread */
    thread = rt_thread_create("lv_demo", lvgl_demo_run, RT_NULL, 1024 * 10, 5, 10);
    if(thread == RT_NULL)
    {
        return RT_ERROR;
    }
    rt_thread_startup(thread);

    return RT_EOK;
}
INIT_APP_EXPORT(lvgl_rtt_demo_init);
