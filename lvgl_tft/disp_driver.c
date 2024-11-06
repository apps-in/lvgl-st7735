/**
 * @file disp_driver.c
 */

#include "disp_driver.h"
#include "disp_spi.h"
#include "esp_lcd_backlight.h"
#include "sdkconfig.h"

void *disp_driver_init(uint32_t gpio_dc, uint32_t gpio_rst, bool use_rst, uint32_t gpio_bckl, bool bckl_pwm, bool bckl_active_low, bool invert_colors, uint8_t display_orientation)
{
    st7735s_setup(gpio_dc, gpio_rst, use_rst, invert_colors, display_orientation);
    st7735s_init();

    const disp_backlight_config_t bckl_config = {
        .gpio_num = gpio_bckl,
        .pwm_control = bckl_pwm,
        .output_invert = bckl_active_low, 
        .timer_idx = 0,
        .channel_idx = 0 // @todo this prevents us from having two PWM controlled displays
    };
    disp_backlight_h bckl_handle = disp_backlight_new(&bckl_config);
    disp_backlight_set(bckl_handle, 100);
    return bckl_handle;
}

void disp_driver_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    st7735s_flush(drv, area, color_map);
}

void disp_driver_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
}

void disp_driver_set_px(lv_disp_drv_t *disp_drv, uint8_t *buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
                        lv_color_t color, lv_opa_t opa)
{
}
