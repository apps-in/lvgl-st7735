/**
 * @file lvgl_spi_conf.h
 *
 */

#ifndef LVGL_SPI_CONF_H
#define LVGL_SPI_CONF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
// DISPLAY PINS
#define DISP_SPI_INPUT_DELAY_NS (0)
#define DISP_SPI_MISO (-1)
#define DISP_SPI_IO2 (-1)
#define DISP_SPI_IO3 (-1)

#define DISP_SPI_HALF_DUPLEX

#define DISP_SPI_TRANS_MODE_SIO

    /**********************
     *      TYPEDEFS
     **********************/

#define SPI_TFT_CLOCK_SPEED_HZ (40 * 1000 * 1000)
#define SPI_TFT_SPI_MODE (0)

    /**********************
     * GLOBAL PROTOTYPES
     **********************/

    /**********************
     *      MACROS
     **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVGL_SPI_CONF_H*/
