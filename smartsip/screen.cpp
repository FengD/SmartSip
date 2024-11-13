#include "screen.h"

TFT_eSPI tft = TFT_eSPI();

void setup_display() {
  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();
  tft.init();
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(TFT_WHITE);
}

void display_image(camera_fb_t *fb) {
  uint8_t* buf = fb->buf;
  uint32_t len = fb->len;
  tft.startWrite();
  tft.setAddrWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  tft.pushColors(buf, len);
  tft.endWrite();
}

bool check_display_is_pressed(void) {
    if(digitalRead(TOUCH_INT) != LOW) {
        delay(1);
        if(digitalRead(TOUCH_INT) != LOW)
        return false;
    }
    return true;
}

void convert_display_xy(uint8_t *x, uint8_t *y) {
    uint8_t x_tmp = *x, y_tmp = *y, _end = 0;
    for(int i = 1; i <= SCREEN_ROTATION; i++){
        x_tmp = *x;
        y_tmp = *y;
        _end = (i % 2) ? SCREEN_WIDTH : SCREEN_HEIGHT;
        *x = y_tmp;
        *y = _end - x_tmp;
    }
}

void get_display_touch_xy(lv_coord_t * x, lv_coord_t * y){
    uint8_t temp[CHSC6X_READ_POINT_LEN] = {0};
    uint8_t read_len = Wire.requestFrom(CHSC6X_I2C_ID, CHSC6X_READ_POINT_LEN);
    if(read_len == CHSC6X_READ_POINT_LEN){
        Wire.readBytes(temp, read_len);
        if (temp[0] == 0x01) {
        convert_display_xy(&temp[2], &temp[4]);
        *x = temp[2];
        *y = temp[4];
        }
    }
}