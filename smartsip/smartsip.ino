#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <esp_camera.h>

#include <ArduinoJson.h>  // for json style text
#include <HardwareSerial.h>  // xiao serial port use

// for web socket display on pad, mobile or PC
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

// Select camera model
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM Do not forget to setup PSRAM as "OPI PSRAM" in "Tools"
#include "camera_pins.h"
#include "config.h"
#include "html_page.h"
#include "file_util.h"
#include "screen.h"
#include "s3_util.h"

const int PIN_XIAO_SD_CARD  = D2;  // sd card on display, if use the sd card slot on XIAO change to 21
const uint8_t unlock[5] = {0x69, 0x01, 0x2d, 0x32, 0x60};
const uint8_t close_water[5] =  {0x69, 0x11, 0x2d, 0x32, 0x70};

bool is_camera_ready = false;
bool is_sd_card_ready = false;
bool is_wifi_ready = false;
bool is_global_time_ready = false;

int image_counter = 0;

// serial port use d6 d7
HardwareSerial serial_port(0);

// Set up server and WebSocket
AsyncWebServer server(80);
WebSocketsServer webSocket(81);

int setup_sdcard() {
  if(!SD.begin(PIN_XIAO_SD_CARD)){
      Serial.println("Card Mount Failed");
      return -1;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return -1;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.println("Setup SD Card Success!");
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  // readFile(SD, "/hello.txt");
  is_sd_card_ready = true;
  return 0;
}

// ---------------------------------------------------------------------------------
// WIFI
void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Loading WIFI not ready!");
    delay(200);
  }
  Serial.print("Setup Wifi Success! Wifi connected with IP: ");
  Serial.println(WiFi.localIP());
  is_wifi_ready = true;
}

// TIME
int setup_time() {
  const char* net_server = "pool.ntp.org";
  const long offset_sec = 0;
  const int day_light_offset_sec = 3600;
  const char *tz = "CST-8";

  configTime(offset_sec, day_light_offset_sec, net_server);
  configTzTime(tz, net_server);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time!");
    return -1;
  }
  Serial.printf("current: %04d%02d%02d_%02d%02d%02d", 
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);
  is_global_time_ready = true;
  return 0;
}

// -------------------------------------------------------------------
// json deserialization
void transform_json2command(const char* json_str) {
  JsonDocument doc;
  deserializeJson(doc, json_str);

  int v = doc["volume"];
  int t = doc["degree"];
  const char* type = doc["type"];

  Serial.println(v);
  Serial.println(t);
  Serial.println(type);

  uint8_t data[5] = {0};
  
  data[0] = 0x69;
  if (v > 500) {
    data[3] = 0x0f;
  } else {
    data[3] = (uint8_t)(v / 10 - 1);
  }

  if (t < 30) {
    // cool water
    data[1] = 0x66;
    data[2] = 0x2d;
  } else if (t >= 30 && t < 55) {
    // warm water
    data[1] = 0x77;
    data[2] = 0x32;
  } else if (t >= 55 && t < 80) {
    // hot water
    data[1] = 0x88;
    data[2] = 0x55;
  } else {
    // boiling water
    data[1] = 0x99;
    data[2] = 0x5e;
  }
  uint16_t checksum = data[1] + data[2] + data[3];
  data[4] = (uint8_t)(checksum & 0xff);
  if (v > 0) {
    if (t > 90) {
      serial_port.write(unlock, sizeof(unlock));
    }
    serial_port.write(data, sizeof(data));
    Serial.printf("command: %x %x %x %x %x\n", data[0], data[1], data[2], data[3], data[4]);
  }
}

// ----------------------------------------------------------------------------------
// call llm server
String call_llm_server(const char* file_path) {
  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String(SERVICE_ADDRESS) + "/smartsip/upload?llm_type=" + String(LLM_TYPE)
                                   + "&image_name=" + String(file_path) 
                                   + "&model=" + String(MODEL);
      http.begin(url);

      int httpResponseCode = http.sendRequest("POST");
      if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("Response: " + response);
          return response;
      } else {
          Serial.println("Error on sending file: " + String(httpResponseCode));
      }
      http.end();
  } else {
      Serial.println("WiFi not connected");
  }
  return "";
}


// --------------------------------------------------------------------------------
// camera
int setup_camera() {
  // Camera pinout
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;  // Reduced XCLK_FREQ_HZ from 20KHz to 10KHz (no EV-VSYNC-OVF message)
  config.frame_size = FRAMESIZE_240X240;  // FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_RGB565;  // PIXFORMAT_JPEG; for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 1;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return -1;
  }
  Serial.println("Camera ready");
  is_camera_ready = true;
  return 0;
}

int get_image_stream() {
  if(is_camera_ready){
    // Take a photo
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Failed to get camera frame buffer");
      return -1;
    }
    char filename[64];
    if (is_global_time_ready) {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        sprintf(filename, "/image_%04d%02d%02d_%02d%02d%02d.jpg", 
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);
      }
    } else {
      sprintf(filename, "/image_%06d.jpg", image_counter);
      image_counter++;
    }
    
    // Save frame to buffer
    size_t out_len = 0;
    uint8_t* out_buf = NULL;
    int jpeg_quality = 30;
    esp_err_t ret = frame2jpg(fb, jpeg_quality, &out_buf, &out_len);
    int stat = -1;
    if (ret == false) {
      Serial.println("JPEG conversion failed");
    } else {
      if (check_display_is_pressed()) {
        Serial.println("display is touched");
        lv_coord_t x, y;
        get_display_touch_xy(&x, &y);
        Serial.println(x);
        Serial.println(y);

        if(y < 120) {
          tft.drawSpot(x, y, 10, TFT_GREEN, TFT_GREEN);
          // Save photo to file
          if (is_sd_card_ready && writeFile(SD, filename, out_buf, out_len) >= 0) {
            Serial.printf("Saved picture: %s\n", filename);
            if (upload_file(filename) == 0) {
              String llm_response = call_llm_server(filename);
              if(llm_response != "") {
                // tft.drawString(llm_response, 10, SCREEN_HEIGHT / 2);
                webSocket.broadcastTXT(llm_response);
                transform_json2command(llm_response.c_str());
                delay(2000);
              }
            }
          }
        } else {
          tft.drawSpot(x, y, 10, TFT_RED, TFT_RED);
          // tft.drawString("Stop", 10, SCREEN_HEIGHT / 2);
          webSocket.broadcastTXT("{\"volume\": \"0\", \"degree\": \"0\", \"type\": \"stop\"}");
          serial_port.write(close_water, sizeof(close_water));
          delay(500);
        }

      }
      free(out_buf);
      stat = 0;
    }

    display_image(fb);

    esp_camera_fb_return(fb);
    return stat;
  }

  return -1;
}


// ----------------------------------------------------------------------

// WebSocket event handler
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.println("Client connected");
  }

  if (type == WStype_TEXT) {
    Serial.printf("Received data: %s\n", payload);
    JsonDocument doc;
    deserializeJson(doc, payload);
    const char* type = doc["type"];
    Serial.printf("Action: %s\n", type); 
    if (strcmp(type, "start") == 0) {
      transform_json2command((char*)(payload));
      webSocket.broadcastTXT(payload);
    } else {
      serial_port.write(close_water, sizeof(close_water));
      webSocket.broadcastTXT("{\"volume\": \"0\", \"degree\": \"0\", \"type\": \"stop\"}");
    }
  }
}

void init_websocket() {
  // WebSocket event handler
  webSocket.onEvent(onWebSocketEvent);
  webSocket.begin();

  // Serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", HTML_PAGE);
  });
  server.begin();
}

// --------------------------------------------------------------------------------


void setup() {
  // Setup Serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Serial Setup Success!");

  // Configure MySerial0 on pins TX=D6 and RX=D7 (-1, -1 means use the default)
  serial_port.begin(9600, SERIAL_8N1, -1, -1);

  // DISPLAY
  setup_display();

  // TEST LED
  pinMode(LED_BUILTIN, OUTPUT);

  // TEST SD Card
  while (setup_sdcard() < 0) {
    Serial.println("Retry Setup SD Card in 1 second!");
    delay(1000);
  }

  // TEST WIFI
  setup_wifi();

  init_websocket();

  // TIME
  if (setup_time() < 0) {
    Serial.println("Setup Global Time Failed, use image_counter!");
    delay(1000);
  }
  
  // TEST CAMERA
  while (setup_camera() < 0) {
    Serial.println("Retry Setup CAMERA in 1 second!");
    delay(1000);
  }
}

void loop() {
  // make an loop action
  webSocket.loop();
  get_image_stream();
  delay(20);
}
