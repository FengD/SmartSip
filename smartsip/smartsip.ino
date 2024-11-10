#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <mbedtls/md.h>
#include <mbedtls/base64.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <esp_camera.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>  // xiao serial port use
#include <Wire.h>  // xiao connect with display by touch action
// for web socket display on pad, mobile or PC
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

// Select camera model
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM Do not forget to setup PSRAM as "OPI PSRAM" in "Tools"
#include "camera_pins.h"

// need to modify ----------------------------------------
// Enter your WiFi credentials
const char *SSID = "";
const char *PWD = "";
const char* ACCESS_KEY = "";
const char* SECRET_KEY = "";
const char* BUCKET_NAME = "";
const char* MINIO_URL = "";  // no need / at the end 
const char* SERVICE_ADDRESS = "";  // no need / at the end
const char* LLM_TYPE = "";
const char* MODEL = "";
// ---------------------------------------------------------

const int PIN_XIAO_SD_CARD  = D2;  // sd card on display, if use the sd card slot on XIAO change to 21
const uint8_t unlock[5] = {0x69, 0x01, 0x2d, 0x32, 0x60};
const uint8_t close_water[5] =  {0x69, 0x11, 0x2d, 0x32, 0x70};

bool is_camera_ready = false;
bool is_sd_card_ready = false;
bool is_wifi_ready = false;
bool is_global_time_ready = false;

int image_counter = 0;

HardwareSerial serial_port(0);

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>SmartSip</title>
  <style>
    body { font-family: Arial, sans-serif; max-width: 600px; margin: auto; padding: 20px; }
    .container { display: flex; flex-direction: column; gap: 20px; }
    .display, .controls { border: 1px solid #ccc; padding: 20px; border-radius: 10px; }
    .display { text-align: center; }
    .display img { width: 100%; height: auto; max-height: 200px; }
    .controls { display: flex; flex-direction: column; gap: 15px; }
    .controls label { display: flex; justify-content: space-between; }
    .progress-container { width: 100%; background-color: #ddd; border-radius: 5px; }
    .progress-bar { width: 0; height: 20px; background-color: #4CAF50; border-radius: 5px; }
    button { width: 100%; padding: 10px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }
    button:active { background-color: #ddd; }
    @media (min-width: 768px) {
      .container { flex-direction: row; gap: 30px; }
      .display, .controls { width: 100%; }
    }
  </style>
</head>
<body>
  <h2>SmartSip</h2>
  <div class="container">
    <div class="display">
      <h3>Current Status</h3>
      <p>Water Temperature(C): <span id="tempDisplay">20</span></p>
      <p>Water Volume(ml): <span id="volumeDisplay">150</span></p>
      <p>Description: <span id="descriptionDisplay">This is the SmartSip</span></p>
      <img id="waterImage" src="https://via.placeholder.com/240x240" alt="Default Image">
    </div>

    <div class="controls">
      <h3>Control Panel</h3>
      <label>Water Volume (100-500ml): <span id="volumeValue">150</span></label>
      <input type="range" id="volume" min="100" max="500" value="150" oninput="updateVolume(this.value)">
      
      <label>Water Temperature (20-100C): <span id="tempValue">20</span></label>
      <input type="range" id="temperature" min="20" max="100" value="20" oninput="updateTemp(this.value)">

      <button onclick="sendCommand('start')">Add Water</button>
      <button onclick="sendCommand('stop')">Stop</button>

    </div>
  </div>
)rawliteral"
R"rawliteral(
  <script>
    const socket = new WebSocket("ws://" + location.hostname + ":81/");
    socket.onmessage = function(event) {
      console.log(event.data);
      const message = JSON.parse(event.data);
      document.getElementById("tempDisplay").textContent = message['degree'];
      document.getElementById("volumeDisplay").textContent = message['volume'];
      document.getElementById("descriptionDisplay").textContent = message['type'];
    };
    function updateVolume(value) {
      document.getElementById("volumeValue").textContent = value;
    }
    function updateTemp(value) {
      document.getElementById("tempValue").textContent = value;
    }
    function sendCommand(command) {
      const volume = document.getElementById("volume").value;
      const temp = document.getElementById("temperature").value;
      const data = { type: command, volume: volume, degree: temp };
      socket.send(JSON.stringify(data));
    }
  </script>
</body>
</html>
)rawliteral";

// Set up server and WebSocket
AsyncWebServer server(80);
WebSocketsServer webSocket(81);

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

uint8_t screen_rotation = 1;

TFT_eSPI tft = TFT_eSPI();

void setup_display() {
  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();
  tft.init();
  tft.setRotation(screen_rotation);
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
    for(int i=1; i<=screen_rotation; i++){
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

// --------------------------------------------------------------------------------
// SD Card
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeMessage(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

int writeFile(fs::FS &fs, const char * path, uint8_t * data, size_t len){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return -1;
    }
    int stat = -1;
    if(file.write(data, len) == len){
        Serial.println("File written");
        stat = 0;
    } else {
        Serial.println("Write failed");
    }
    file.close();
    return stat;
}

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

  // test sd card utils
  // listDir(SD, "/", 0);
  // createDir(SD, "/mydir");
  // listDir(SD, "/", 0);
  // removeDir(SD, "/mydir");
  // listDir(SD, "/", 2);
  // writeMessage(SD, "/hello.txt", "Hello ");
  // appendFile(SD, "/hello.txt", "World!\n");
  // readFile(SD, "/hello.txt");
  // deleteFile(SD, "/foo.txt");
  // renameFile(SD, "/hello.txt", "/foo.txt");
  // readFile(SD, "/foo.txt");
  // testFileIO(SD, "/test.txt");
  is_sd_card_ready = true;
  return 0;
}

// -----------------------------------------------------------------------------------
String get_date_rf2616_format() {
  time_t now;
  struct tm timeinfo;
  char buf[64];

  time(&now);
  gmtime_r(&now, &timeinfo);
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
  
  return String(buf);
}

// ---------------------------------------------------------------------------------
// LED
void flash_led() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200); 
  }
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

// --------------------------------------------------------------------------
// S3 upload
String generate_signature(String method, String date, String object_name) {
    String stringToSign = method + "\n\n" + "image/jpeg" + "\n" + date + "\n" + "/" + BUCKET_NAME + object_name;
    unsigned char hmacResult[20];  // SHA1 output 160bit（20 Byte）
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, info, 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)SECRET_KEY, strlen(SECRET_KEY));
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)stringToSign.c_str(), stringToSign.length());
    mbedtls_md_hmac_finish(&ctx, hmacResult);
    mbedtls_md_free(&ctx);

    char base64Result[64];
    size_t outlen;
    mbedtls_base64_encode((unsigned char*)base64Result, 64, &outlen, hmacResult, 20);

    return String(base64Result);
}

int upload_file(const char* file_path) {
  int status = -1;
  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String(MINIO_URL) + "/" + String(BUCKET_NAME) + String(file_path);
      http.begin(url);

      String date = get_date_rf2616_format();
      String signature = generate_signature("PUT", date, String(file_path));

      http.addHeader("Authorization", "AWS " + String(ACCESS_KEY) + ":" + signature);
      http.addHeader("Date", date);
      http.addHeader("Content-Type", "image/jpeg");

      File file = SD.open(file_path, "r");
      if (!file) {
          Serial.println("Failed to open file for reading");
          return status;
      }

      int httpResponseCode = http.sendRequest("PUT", &file, file.size());
      if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("Response: " + response);
          status = 0;
      } else {
          Serial.println("Error on sending file: " + String(httpResponseCode));
      }

      file.close();
      http.end();
  } else {
      Serial.println("WiFi not connected");
  }

  return status;
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
    request->send_P(200, "text/html", htmlPage);
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
  // make an action
  webSocket.loop();
  get_image_stream();
  delay(20);
}
