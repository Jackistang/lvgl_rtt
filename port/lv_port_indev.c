/**
 * @file lv_port_indev_templ.c
 *
 */
#include "lv_conf.h"
 /*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if LV_USE_INDEV

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"


#ifdef LV_USE_TOUCH
#include <touch.h>

static void touchpad_init(void);
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

static rt_device_t touch_dev;
#endif /* LV_USE_TOUCH */



void lv_port_indev_init(void)
{
    /* Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */


    lv_indev_drv_t indev_drv;

    /*------------------
     * Touchpad
     * -----------------*/
#ifdef LV_USE_TOUCH
    /*Initialize your touchpad if you have*/
    touchpad_init();

    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);
#endif  /* LV_USE_TOUCH */

}

/**********************
 *   STATIC FUNCTIONS
 **********************/


#ifdef LV_USE_TOUCH
/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    /*Your code comes here*/
    touch_dev = rt_device_find(LV_TOUCH_NAME);
    RT_ASSERT(touch_dev != RT_NULL);
    rt_device_open(touch_dev, RT_DEVICE_FLAG_RDONLY);
}

/* Will be called by the library to read the touchpad */
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    /*Save the pressed coordinates and the state*/
    struct rt_touch_data touch_data;
    if (rt_device_read(touch_dev, 0, &touch_data, 1) == 1) {
        last_x = touch_data.x_coordinate;
        last_y = touch_data.y_coordinate;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;

    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}
#endif  /* LV_USE_TOUCH */

#else /* LV_USE_INDEV */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
