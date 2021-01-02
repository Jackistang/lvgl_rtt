/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#if LV_USE_GPU_STM32_DMA2D
    #include "../lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.h"
#endif

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>

#if defined BSP_USING_LCD
#include "lcd_port.h"
#elif #error "ART-PI not select BSP_USING_LCD"
#endif
/*********************
 *      DEFINES
 *********************/
#ifdef LV_USE_AXI_SRAM
#define DISPLAY_BUFFER_LINES       40  // 缓存的行数
#endif
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

#if LV_USE_GPU_STM32_DMA2D
static void disp_wait_cb(struct _disp_drv_t * disp_drv);
#endif

#if LV_USE_GPU
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
        const lv_area_t * fill_area, lv_color_t color);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static struct drv_lcd_device *lcd;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed your display drivers `flush_cb` to copy its content to your dispay.
     * The buffer has to be greater than 1 display row
     *
     * There are three buffering configurations:
     * 1. Create ONE buffer with some rows: 
     *      LVGL will draw the display's content here and writes it to your display
     * 
     * 2. Create TWO buffer with some rows: 
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     * 
     * 3. Create TWO screen-sized buffer: 
     *      Similar to 2) but the buffer have to be screen sized. When LVGL is ready it will give the
     *      whole frame to display. This way you only need to change the frame buffer's address instead of
     *      copying the pixels.
     * */
    static lv_disp_buf_t draw_buf_dsc;

#ifdef LV_USE_AXI_SRAM
    static lv_color_t draw_buf_1[LV_HOR_RES_MAX * DISPLAY_BUFFER_LINES];  /*A buffer for 40 rows in AXI SRAM*/
    static lv_color_t draw_buf_2[LV_HOR_RES_MAX * DISPLAY_BUFFER_LINES];  /*An other buffer for 40 rows in AXI SRAM*/
    lv_disp_buf_init(&draw_buf_dsc, draw_buf_1, draw_buf_2, LV_HOR_RES_MAX * DISPLAY_BUFFER_LINES);   /*Initialize the display buffer*/
#else
    lv_color_t *draw_buf_1 = (lv_color_t *)rt_malloc(LV_HOR_RES_MAX * LV_VER_RES_MAX * sizeof(lv_color_t));
    lv_color_t *draw_buf_2 = (lv_color_t *)rt_malloc(LV_HOR_RES_MAX * LV_VER_RES_MAX * sizeof(lv_color_t));
    lv_disp_buf_init(&draw_buf_dsc, draw_buf_1, draw_buf_2, LV_HOR_RES_MAX * LV_VER_RES_MAX);
#endif

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = lcd->lcd_info.width;
    disp_drv.ver_res = lcd->lcd_info.height;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

#if LV_USE_GPU_STM32_DMA2D
    disp_drv.wait_cb = disp_wait_cb;
#endif

    /*Set a display buffer*/
    disp_drv.buffer = &draw_buf_dsc;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
    /*You code here*/
    rt_err_t ret;
    lcd = (struct drv_lcd_device *)rt_device_find("lcd");
    RT_ASSERT(lcd);

    ret = rt_device_open((rt_device_t)lcd, RT_DEVICE_FLAG_RDWR);
    RT_ASSERT(ret == RT_EOK);

#if LV_USE_GPU_STM32_DMA2D
    lv_gpu_stm32_dma2d_init();
#elif LV_USE_CUSTOM_DMA2D
    __HAL_RCC_DMA2D_CLK_ENABLE();
    NVIC_EnableIRQ(DMA2D_IRQn);
#endif
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
#if LV_USE_CUSTOM_DMA2D == 0
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

    lv_color_t *buf = (lv_color_t *)lcd->front_buf;

#if LV_USE_GPU_STM32_DMA2D
    buf = buf + area->y1 * disp_drv->hor_res + area->x1;
    lv_coord_t w = (area->x2 - area->x1 + 1);
    lv_coord_t h = (area->y2 - area->y1 + 1);
    lv_gpu_stm32_dma2d_copy(buf, disp_drv->hor_res, color_p, w, w, h);
#else
    int32_t x;
    int32_t y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            *(buf + y*disp_drv->hor_res + x) = *color_p;
            color_p++;
        }
    }
#endif

    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}
#else
DMA2D_HandleTypeDef hdma2d;
static lv_disp_drv_t *disp_drv_p;
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    uint8_t *p = (uint8_t *)lcd->front_buf;

    int16_t w = (area->x2 - area->x1 + 1);
    int16_t h = (area->y2 - area->y1 + 1);

    disp_drv_p = disp_drv;
    /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
    hdma2d.Init.Mode         = DMA2D_M2M;
    hdma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565;
    hdma2d.Init.OutputOffset = (LV_HOR_RES_MAX - w);

    /*##-3- Foreground Configuration ###########################################*/
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0xFF;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565 ;
    hdma2d.LayerCfg[1].InputOffset = 0;

    hdma2d.Instance          = DMA2D;

    /*##-4- DMA2D Initialization ###############################################*/
    if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    /*##-5- Start DMA2D transfer ###############################################*/

    if (HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)color_p, (uint32_t)(p) + 2 * (LV_HOR_RES_MAX * area->y1 + area->x1), w, h) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
}
void DMA2D_IRQHandler(void)
{
    rt_enter_critical();
    /* USER CODE BEGIN DMA2D_IRQn 0 */
    lv_disp_flush_ready(disp_drv_p);
    /* USER CODE END DMA2D_IRQn 0 */
    HAL_DMA2D_IRQHandler(&hdma2d);
    /* USER CODE BEGIN DMA2D_IRQn 1 */

    /* USER CODE END DMA2D_IRQn 1 */
    rt_exit_critical();
}
#endif

#if LV_USE_GPU_STM32_DMA2D
/** OPTIONAL: Called periodically while lvgl waits for operation to be completed.
 * For example flushing or GPU
 * User can execute very simple tasks here or yield the task */
static void disp_wait_cb(struct _disp_drv_t * disp_drv)
{
    rt_thread_mdelay(1);
}
#endif

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
