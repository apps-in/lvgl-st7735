/**
 * @file lvgl_helpers.h
 */

#ifndef LVGL_HELPERS_H
#define LVGL_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>

#include "lvgl_spi_conf.h"
#include "lvgl_tft/disp_driver.h"
#include "lvgl_tft/esp_lcd_backlight.h"
#include "hal/spi_hal.h"
#include "esp_idf_version.h"
/*********************
 *      DEFINES
 *********************/

#if CONFIG_IDF_TARGET_ESP32S3 
    #define DMA_MAX_BIT_LENGHT (1<<18) // according with SPI_LL_DMA_MAX_BIT_LEN in spi_ll.h
#else
    #define DMA_MAX_BIT_LENGHT (1<<24) // according with SPI_LL_DMA_MAX_BIT_LEN in spi_ll.h
#endif

/**********************
 *      TYPEDEFS
 **********************/

typedef struct
{
    int lv_hor_res_max;
    int lv_ver_res_max;
    int spi_host; 
    int spi_mosi; 
    int spi_clk; 
    int spi_cs;
    int dc;
    int rst;
    bool use_rst;
    int bckl; 
    bool bckl_pwm; 
    bool bckl_active_low; 
    bool invert_colors; 
    uint8_t display_orientation;
} lvgl_st7735_settings_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lvgl_i2c_locking(void* leader);

/* Initialize detected SPI and I2C bus and devices */
void lvgl_driver_init(lvgl_st7735_settings_t settings);

uint32_t lvgl_driver_get_display_buffer_size();

/* Initialize SPI master  */
bool lvgl_spi_driver_init(int host, int miso_pin, int mosi_pin, int sclk_pin,
    int max_transfer_sz, int dma_channel, int quadwp_pin, int quadhd_pin);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LVGL_HELPERS_H */
