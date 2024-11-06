/**
 * @file lvgl_helpers.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "sdkconfig.h"
#include "lvgl_helpers.h"
#include "esp_log.h"

#include "lvgl_tft/disp_spi.h"

#include "lvgl_spi_conf.h"

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

#define TAG "lvgl_helpers"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static uint32_t tft_display_buffer_size;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* Interface and driver initialization */
void lvgl_driver_init(lvgl_st7735_settings_t settings)
{

    uint32_t tft_display_buffer_size_in_bits = settings.lv_hor_res_max * 40 * 3 * 8;
    tft_display_buffer_size = (tft_display_buffer_size_in_bits > DMA_MAX_BIT_LENGHT) ? ((DMA_MAX_BIT_LENGHT - 1000) / 8) / 3 : settings.lv_hor_res_max * 40;

    ESP_LOGI(TAG, "Display buffer size: %ld", tft_display_buffer_size);

    /* Display controller initialization */
    ESP_LOGI(TAG, "Initializing SPI master for display");

    lvgl_spi_driver_init(settings.spi_host,
                         DISP_SPI_MISO, settings.spi_mosi, settings.spi_clk,
                         tft_display_buffer_size * 2, SPI_DMA_CH_AUTO,
                         DISP_SPI_IO2, DISP_SPI_IO3);

    disp_spi_add_device(settings.spi_host, settings.spi_cs);

    disp_driver_init(settings.dc,
                     settings.rst,
                     settings.use_rst,
                     settings.bckl,
                     settings.bckl_pwm,
                     settings.bckl_active_low,
                     settings.invert_colors,
                     settings.display_orientation);
}

uint32_t lvgl_driver_get_display_buffer_size()
{
    return tft_display_buffer_size;
}

/* Initialize spi bus master
 *
 * NOTE: dma_chan type and value changed to int instead of spi_dma_chan_t
 * for backwards compatibility with ESP-IDF versions prior v4.3.
 *
 * We could use the ESP_IDF_VERSION_VAL macro available in the "esp_idf_version.h"
 * header available since ESP-IDF v4.
 */
bool lvgl_spi_driver_init(int host,
                          int miso_pin, int mosi_pin, int sclk_pin,
                          int max_transfer_sz,
                          int dma_channel,
                          int quadwp_pin, int quadhd_pin)
{
    assert((0 <= host) && (SPI_HOST_MAX > host));
    const char *spi_names[] = {
        "SPI1_HOST", "SPI2_HOST", "SPI3_HOST"};

    ESP_LOGI(TAG, "Configuring SPI host %s", spi_names[host]);
    ESP_LOGI(TAG, "MISO pin: %d, MOSI pin: %d, SCLK pin: %d, IO2/WP pin: %d, IO3/HD pin: %d",
             miso_pin, mosi_pin, sclk_pin, quadwp_pin, quadhd_pin);

    ESP_LOGI(TAG, "Max transfer size: %d (bytes)", max_transfer_sz);

    spi_bus_config_t buscfg = {
        .miso_io_num = miso_pin,
        .mosi_io_num = mosi_pin,
        .sclk_io_num = sclk_pin,
        .quadwp_io_num = quadwp_pin,
        .quadhd_io_num = quadhd_pin,
        .max_transfer_sz = max_transfer_sz};

    ESP_LOGI(TAG, "Initializing SPI bus...");

    esp_err_t ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);

    return ESP_OK != ret;
}
