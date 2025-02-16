/**
 * @file st7735s.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "st7735s.h"
#include "disp_spi.h"
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*********************
 *      DEFINES
 *********************/
#define TAG "ST7735S"

/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct
{
	uint8_t cmd;
	uint8_t data[16];
	uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void st7735s_send_cmd(uint8_t cmd);
static void st7735s_send_data(void *data, uint16_t length);
static void st7735s_send_color(void *data, uint16_t length);
static void st7735s_set_orientation(uint8_t orientation);

/**********************
 *  STATIC VARIABLES
 **********************/
uint8_t st7735s_portrait_mode = 0;

uint32_t st7735S_dc;

uint32_t st7735S_rst;
bool st7735S_use_rst;

bool st7735S_invert_colors;

uint8_t lv_display_orientation;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void st7735s_setup(uint32_t gpio_dc, uint32_t gpio_rst, bool use_rst, bool invert_colors, uint8_t display_orientation)
{
	st7735S_dc = gpio_dc;
	st7735S_rst = gpio_rst;
	st7735S_use_rst = use_rst;
	st7735S_invert_colors = invert_colors;
	lv_display_orientation = display_orientation;
}

void st7735s_init(void)
{

	lcd_init_cmd_t init_cmds[] = {
		{ST7735_SWRESET, {0}, 0x80},							   // Software reset, 0 args, w/delay 150
		{ST7735_SLPOUT, {0}, 0x80},								   // Out of sleep mode, 0 args, w/delay 500
		{ST7735_FRMCTR1, {0x01, 0x2C, 0x2D}, 3},				   // Frame rate ctrl - normal mode, 3 args: Rate = fosc/(1x2+40) * (LINE+2C+2D)
		{ST7735_FRMCTR2, {0x01, 0x2C, 0x2D}, 3},				   // Frame rate control - idle mode, 3 args:Rate = fosc/(1x2+40) * (LINE+2C+2D)
		{ST7735_FRMCTR3, {0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D}, 6}, // Frame rate ctrl - partial mode, 6 args:Dot inversion mode. Line inversion mode
		{ST7735_INVCTR, {0x07}, 1},								   // Display inversion ctrl, 1 arg, no delay:No inversion
		{ST7735_PWCTR1, {0xA2, 0x02, 0x84}, 3},					   // Power control, 3 args, no delay:-4.6V AUTO mode
		{ST7735_PWCTR2, {0xC5}, 1},								   // Power control, 1 arg, no delay:VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
		{ST7735_PWCTR3, {0x0A, 0x00}, 2},						   // Power control, 2 args, no delay: Opamp current small, Boost frequency
		{ST7735_PWCTR4, {0x8A, 0x2A}, 2},						   // Power control, 2 args, no delay: BCLK/2, Opamp current small & Medium low
		{ST7735_PWCTR5, {0x8A, 0xEE}, 2},						   // Power control, 2 args, no delay:
		{ST7735_VMCTR1, {0x0E}, 1},								   // Power control, 1 arg, no delay:
		{st7735S_invert_colors ? ST7735_INVON :ST7735_INVOFF, {0}, 0}, // set non-inverted mode
		{ST7735_COLMOD, {0x05}, 1},																								// set color mode, 1 arg, no delay: 16-bit color
		{ST7735_GMCTRP1, {0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10}, 16}, // 16 args, no delay:
		{ST7735_GMCTRN1, {0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10}, 16}, // 16 args, no delay:
		{ST7735_NORON, {0}, TFT_INIT_DELAY},																					// Normal display on, no args, w/delay 10 ms delay
		{ST7735_DISPON, {0}, TFT_INIT_DELAY},																					// Main screen turn on, no args w/delay 100 ms delay
		{0, {0}, 0xff}};

	// Initialize non-SPI GPIOs
	gpio_pad_select_gpio(st7735S_dc);
	gpio_set_direction(st7735S_dc, GPIO_MODE_OUTPUT);

	if (st7735S_use_rst)
	{
		gpio_pad_select_gpio(st7735S_rst);
		gpio_set_direction(st7735S_rst, GPIO_MODE_OUTPUT);

		// Reset the display
		gpio_set_level(st7735S_rst, 0);
		vTaskDelay(100 / portTICK_DELAY_MS);
		gpio_set_level(st7735S_rst, 1);
		vTaskDelay(100 / portTICK_DELAY_MS);
	}

	ESP_LOGI(TAG, "ST7735S initialization.");

	// Send all the commands
	uint16_t cmd = 0;
	while (init_cmds[cmd].databytes != 0xff)
	{
		st7735s_send_cmd(init_cmds[cmd].cmd);
		st7735s_send_data(init_cmds[cmd].data, init_cmds[cmd].databytes & 0x1F);
		if (init_cmds[cmd].databytes & 0x80)
		{
			vTaskDelay(100 / portTICK_DELAY_MS);
		}
		cmd++;
	}

	if ((lv_display_orientation == 0) || (lv_display_orientation == 1))
	{
		st7735s_portrait_mode = 1;
	}
	else
	{
		st7735s_portrait_mode = 0;
	}

	st7735s_set_orientation(lv_display_orientation);
}

void st7735s_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
	uint8_t data[4];

	/*Column addresses*/
	st7735s_send_cmd(0x2A);
	data[0] = (area->x1 >> 8) & 0xFF;
	data[1] = (area->x1 & 0xFF) + (st7735s_portrait_mode ? COLSTART : ROWSTART);
	data[2] = (area->x2 >> 8) & 0xFF;
	data[3] = (area->x2 & 0xFF) + (st7735s_portrait_mode ? COLSTART : ROWSTART);
	st7735s_send_data(data, 4);

	/*Page addresses*/
	st7735s_send_cmd(0x2B);
	data[0] = (area->y1 >> 8) & 0xFF;
	data[1] = (area->y1 & 0xFF) + (st7735s_portrait_mode ? ROWSTART : COLSTART);
	data[2] = (area->y2 >> 8) & 0xFF;
	data[3] = (area->y2 & 0xFF) + (st7735s_portrait_mode ? ROWSTART : COLSTART);
	st7735s_send_data(data, 4);

	/*Memory write*/
	st7735s_send_cmd(0x2C);

	uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);
	st7735s_send_color((void *)color_map, size * 2);
}

void st7735s_sleep_in()
{
	st7735s_send_cmd(0x10);
}

void st7735s_sleep_out()
{
	st7735s_send_cmd(0x11);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void st7735s_send_cmd(uint8_t cmd)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(st7735S_dc, 0); /*Command mode*/
	disp_spi_send_data(&cmd, 1);
}

static void st7735s_send_data(void *data, uint16_t length)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(st7735S_dc, 1); /*Data mode*/
	disp_spi_send_data(data, length);
}

static void st7735s_send_color(void *data, uint16_t length)
{
	disp_wait_for_pending_transactions();
	gpio_set_level(st7735S_dc, 1); /*Data mode*/
	disp_spi_send_colors(data, length);
}

static void st7735s_set_orientation(uint8_t orientation)
{
	const char *orientation_str[] = {
		"PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"};

	ESP_LOGD(TAG, "Display orientation: %s", orientation_str[orientation]);

	/*
		Portrait:  0xC8 = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_BGR
		Landscape: 0xA8 = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_BGR
		Remark: "inverted" is ignored here
	*/
	uint8_t data[] = {0xC8, 0xC8, 0xA8, 0xA8};

	ESP_LOGD(TAG, "0x36 command value: 0x%02X", data[orientation]);

	st7735s_send_cmd(ST7735_MADCTL);
	st7735s_send_data((void *)&data[orientation], 1);
}
