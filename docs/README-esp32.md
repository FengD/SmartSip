# Brief

The code is in the `smartsip` folder. Open it with Arduino.

Do not forget to change the constant to yours.


``` c
const char *SSID = "";
const char *PWD = "";
const char* ACCESS_KEY = "";
const char* SECRET_KEY = "";
const char* BUCKET_NAME = "";
const char* MINIO_URL = "";
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
* Library manager: `I2C_BM8563_RTC -> 1.0.4`, `ArduinoHttpClient -> 0.6.1`, `Seeed_Arduino_mbedtls -> 3.0.2`, `lvgl -> 9.2.0`, `TFT_eSPI -> 2.5.43`
