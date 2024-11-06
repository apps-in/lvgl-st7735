# ESP32 LVGL ST7735 driver

# Define sources and include dirs
COMPONENT_SRCDIRS := . lvgl_tft 
COMPONENT_ADD_INCLUDEDIRS := .

# LVGL is supposed to be used as a ESP-IDF component
# -> lvlg is already in the include path
# -> we use simple include
CFLAGS += -DLV_LVGL_H_INCLUDE_SIMPLE

# TFT display drivers
COMPONENT_ADD_INCLUDEDIRS += lvgl_tft
