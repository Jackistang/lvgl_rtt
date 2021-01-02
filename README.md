# lvgl_rtt
lvgl package for RT-Thread



两个提高LVGL速度的诀窍，第一，把缓存放在AXI SRAM里面而不是SDRAM里面，第二，使用MDMA复制数据。

感谢 “ART-Pi DIY 开源硬件群(1016035998)” 里的 **斌**，**阿达 \^_^**，**Mzx**，**手把手** 的讨论。



需制作成选项的：

```C
#define LV_USE_CUSTOM_DMA2D  1
// 使用时在 stm32h7xx_hal_conf.h 里打开 `#define HAL_DMA2D_MODULE_ENABLED ` 的注释

#define LV_USE_AXI_SRAM
// 是否使用 AXI SRAM

#define LV_USE_INDEV
#define LV_USE_TOUCH
#define LV_TOUCH_NAME	"gt1151"

/* 1: Enable file system (might be required for images */
#define LV_USE_FILESYSTEM       1

/*1: Show CPU usage and FPS count in the right bottom corner*/
#define LV_USE_PERF_MONITOR     0

/* Color depth:
 * - 1:  1 byte per pixel
 * - 8:  RGB332
 * - 16: RGB565
 * - 32: ARGB8888
 */
#define LV_COLOR_DEPTH	16

/* Maximal horizontal and vertical resolution to support by the library.*/
#define LV_HOR_RES_MAX          (480)
#define LV_VER_RES_MAX          (320)

/* Dot Per Inch: used to initialize default sizes.
 * E.g. a button with width = LV_DPI / 2 -> half inch wide
 * (Not so important, you can adjust it to modify default sizes and spaces)*/
#define LV_DPI              130
```

更改 lcd_port.h 里的 LCD 驱动参数





```C
rt_size_t touch_gt1151_readpoint(struct rt_touch_device *touch, void *data_buf, rt_size_t touch_num)
{
    RT_ASSERT(touch_num == 1);  // only support 1 touch point

    struct gt1151_object *object = (struct gt1151_object *)touch->config.user_data;

    uint8_t status;
    uint8_t buf[8];
    uint16_t x = 0, y = 0;

    struct rt_touch_data *pdata = (struct rt_touch_data *)data_buf;

    gt1151_read_regs(object, GT1151_STATUS, &status, 1);

    if (status < 0x80)  // no data get
        return 0;

    else if (status == 0x80) { // no data get
        LOG_D("status: %x", status);
        gt1151_clear_status(object);
        return 0;
    }
    else {
        gt1151_clear_status(object);

        LOG_D("status: %x", status);

        gt1151_read_regs(object, GT1151_STATUS, buf, 8);

        x = ((uint16_t)buf[3] << 8) + buf[2];
        y = ((uint16_t)buf[5] << 8) + buf[4];

        if (x > GT1151_TOUCH_WIDTH - 1)  x = GT1151_TOUCH_WIDTH;
        if (y > GT1151_TOUCH_HEIGHT - 1) y = GT1151_TOUCH_HEIGHT;

        pdata->x_coordinate = x;
        pdata->y_coordinate = y;
        pdata->track_id = 1;
        pdata->event = RT_TOUCH_EVENT_DOWN;
        pdata->timestamp = rt_touch_get_ts();

        status = 0;

        LOG_D("\t x: %d, y: %d", x, y);
    }
    return 1;
}
```

