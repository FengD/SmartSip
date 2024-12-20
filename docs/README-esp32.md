# Brief

The code is in the `smartsip` folder. Open it with Arduino.

Do not forget to change the constant to yours.
create a file named `config.cpp`, include `config.h` and gives the info below.

``` c
const char *SSID = "";
const char *PWD = "";
const char* ACCESS_KEY = "";
const char* SECRET_KEY = "";
const char* BUCKET_NAME = "";
const char* MINIO_URL = "";
const char* SERVICE_ADDRESS = "";  // backend service address of the backend
const char* LLM_TYPE = "";   // openai or ollama now suppert
const char* MODEL = "gpt-4o";  // gpt-4o etc for openai, llava for ollama
```

# WIFI
* the device only support 2.4G wifi.

# Camera
* in the tab `Tools->PSRAM` change to `ORI PSRAM` to avoid camera errors.

# Minio
* Get binare at https://dl.min.io/server/minio/release/linux-amd64/minio (for linux x86), if you use any other platform, download other binare https://dl.min.io/server/minio/release/ .
    * `chmod +x minio`
    * `./minio server [data_folder_path]`
* Create a pair of `ACCESS_KEY` and its `SECRET_KEY`

# SD Card
* Format the SD Card as `FAT32`(recommand) or `ext4`.

# Arduino Libraries
* Navigate to `"File->Preferences->Settings"`, and fill `"Additional Boards Manager URLs"` with the url below: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
* Library manager:
     * `I2C_BM8563_RTC -> 1.0.4(optional)`
     * `ArduinoHttpClient -> 0.6.1` (for http request)
     * `Seeed_Arduino_mbedtls -> 3.0.2` (for base64 encoder)
     * `lvgl -> 9.2.0` (should use the version of Seeed： https://github.com/Seeed-Projects/SeeedStudio_lvgl )
     * `TFT_eSPI -> 2.5.43` (should use the version of Seeed: https://github.com/Seeed-Projects/SeeedStudio_TFT_eSPI )
     * follow this link to test the display https://wiki.seeedstudio.com/get_start_round_display/ especially the `lv_conf.h`
     * `ArduinoJson -> 7.2.0` (for json use)
     * `Async TCP -> 3.2.14` (by Me-No-Dev)t
     * `ESP Async WebServer -> 3.3.22` (by Me-No-Dev)
     * `WebSockets -> 2.6.1`(by Markus Sattler)
     * QRcode_eSPI -> 2.0.0 (by Ruslan Baybekow and Jose)
     * QRCodeGenerator -> 0.0.1 (by Felix Erdmann)
     * base64 -> 1.3.0 (by Densaugeo)
