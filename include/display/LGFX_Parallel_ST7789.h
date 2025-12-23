#ifndef LGFX_PARALLEL_ST7789_HPP
#define LGFX_PARALLEL_ST7789_HPP

#if defined(LILYGO_T_DISPLAY_S3)

#include <LovyanGFX.hpp>

#include "esp32s3.h"

#define TFT_WIDTH 170
#define TFT_HEIGHT 320
#define OFFSET_X 35
#define OFFSET_Y 0

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789    _panel;
  lgfx::Bus_Parallel8   _bus;
  lgfx::Light_PWM       _light;

public:
  LGFX(void) {
    auto cfg = _bus.config();

    cfg.freq_write = 20000000;
    cfg.pin_wr     = PIN_LCD_WR;
    cfg.pin_rd     = PIN_LCD_RD;
    cfg.pin_rs     = PIN_LCD_DC;
    cfg.pin_d0     = PIN_LCD_D0;
    cfg.pin_d1     = PIN_LCD_D1;
    cfg.pin_d2     = PIN_LCD_D2;
    cfg.pin_d3     = PIN_LCD_D3;
    cfg.pin_d4     = PIN_LCD_D4;
    cfg.pin_d5     = PIN_LCD_D5;
    cfg.pin_d6     = PIN_LCD_D6;
    cfg.pin_d7     = PIN_LCD_D7;

    _bus.config(cfg);
    _panel.setBus(&_bus);

    auto pcfg = _panel.config();

    pcfg.pin_cs        = PIN_LCD_CS;
    pcfg.pin_rst       = PIN_LCD_RES;
    pcfg.pin_busy      = -1;
    pcfg.memory_width  = TFT_WIDTH;
    pcfg.memory_height = TFT_HEIGHT;
    pcfg.panel_width   = TFT_WIDTH;
    pcfg.panel_height  = TFT_HEIGHT;
    pcfg.offset_x      = OFFSET_X;
    pcfg.offset_y      = OFFSET_Y;
    pcfg.offset_rotation = 0;
    pcfg.dummy_read_pixel = 8;
    pcfg.dummy_read_bits  = 1;
    pcfg.readable         = false;
    pcfg.invert          = true;
    pcfg.rgb_order       = false;
    pcfg.dlen_16bit      = false;
    pcfg.bus_shared      = false;

    _panel.config(pcfg);

    auto lcfg = _light.config();

    lcfg.pin_bl = -1;
    lcfg.invert = false;
    lcfg.freq   = 44100;
    lcfg.pwm_channel = 7;

    _light.config(lcfg);
    _panel.setLight(&_light);

    setPanel(&_panel);
  }
};

#endif //LILYGO_T_DISPLAY_S3
#endif //LGFX_PARALLEL_ST7789_HPP
