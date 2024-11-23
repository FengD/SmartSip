#ifndef SCREEN_H_
#define SCREEN_H_

#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Wire.h>  // xiao connect with display by touch action
#include <esp_camera.h>

// display
#define TOUCH_INT D7
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define CHSC6X_READ_POINT_LEN 5
#define CHSC6X_I2C_ID 0x2e

// 0,1,2,3  -> 0, 90, 180, 270 clockwise
// when typeC port on XIAO is to the top
// set rotation to 1
// |-----------------------------------------|
// |(WIDTH_MAX, Height_MAX)------------------|
// |-----------------------------------------|
// |-----------------------------------------|
// |-----------------------------------------|
// |-----------------------------------------|
// |------------------(WIDTH_MIN, HEIGHT_MIN)|

const uint8_t SCREEN_ROTATION = 0;

extern TFT_eSPI tft;

void setup_display();

void display_image(camera_fb_t *fb);

bool check_display_is_pressed(void);

void get_display_touch_xy(lv_coord_t * x, lv_coord_t * y);

#endif