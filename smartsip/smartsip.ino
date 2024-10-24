#include <WiFi.h>
#include <time.h>
#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Select camera model
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// Enter your WiFi credentials
const char *SSID = "Feng";
const char *PWD = "qwertyuiop1";

bool is_camera_ready = false;
bool is_sd_card_ready = false;
bool is_wifi_ready = false;
bool is_global_time_ready = false;
const int PIN_XIAO_SD_CARD  = 21;

int image_counter = 0;

// -----------------------------------------------
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

// LED
void flash_led() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200); 
  }
}

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
  config.xclk_freq_hz = 10000000; // Reduced XCLK_FREQ_HZ from 20KHz to 10KHz (no EV-VSYNC-OVF message)
  //config.frame_size = FRAMESIZE_UXGA;
  //config.frame_size = FRAMESIZE_SVGA;
  config.frame_size = FRAMESIZE_UXGA;
  //config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.pixel_format = PIXFORMAT_RGB565;
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

int take_picture() {
  if( is_sd_card_ready && is_camera_ready){
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
    
    // Save photo to file
    size_t out_len = 0;
    uint8_t* out_buf = NULL;
    int jpeg_quality = 30;
    esp_err_t ret = frame2jpg(fb, jpeg_quality, &out_buf, &out_len);
    int stat = -1;
    if (ret == false) {
      Serial.println("JPEG conversion failed");
    } else {
      // Save photo to file
      if (writeFile(SD, filename, out_buf, out_len) >= 0) {
        Serial.printf("Saved picture: %s\n", filename);
      }
      free(out_buf);
      stat = 0;
    }
    esp_camera_fb_return(fb);
    return stat;
  }

  return -1;
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


void setup() {
  // Setup Serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Serial Setup Success!");

  // TEST LED
  pinMode(LED_BUILTIN, OUTPUT);

  // TEST SD Card
  while (setup_sdcard() < 0) {
    Serial.println("Retry Setup SD Card in 1 second!");
    delay(1000);
  }

  // TEST WIFI
  setup_wifi();

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
  if (take_picture() >= 0) {
    flash_led();
  }
  delay(3000);
}
