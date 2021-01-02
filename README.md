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



## ART-PI 使用该软件包

首先在软件包中选中 BSP 中提供的 LCD 驱动，在

```
Hardware Drivers Config -->
	On-chip Peripheral -->
		[*]Enable LCD
```



修改 lcd_port.h 里的显示驱动参数来适配屏幕（以正点原子4384显示屏为例），参数修改为如下：

```C
#define LCD_WIDTH           800
#define LCD_HEIGHT          480
#define LCD_BITS_PER_PIXEL  16//24
#define LCD_BUF_SIZE        (LCD_WIDTH * LCD_HEIGHT * LCD_BITS_PER_PIXEL / 8)
#define LCD_PIXEL_FORMAT    RTGRAPHIC_PIXEL_FORMAT_RGB565//888

#define LCD_HSYNC_WIDTH     1
#define LCD_VSYNC_HEIGHT    1
#define LCD_HBP             40//88
#define LCD_VBP             32
#define LCD_HFP             48//40
#define LCD_VFP             13
```

在软件包管理器中选中 lvgl ，并进行相应配置。（详细描述）