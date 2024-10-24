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
* in the tab 'Tools->PSRAM' change to `ORI PSRAM` to avoid camera errors.

# Minio
* Get binare at https://dl.min.io/server/minio/release/linux-amd64/minio (for linux x86), if you use any other platform, download other binare https://dl.min.io/server/minio/release/ .
    * `chmod +x minio`
    * `./minio server [data_folder_path]`
* Create a pair of `ACCESS_KEY` and its `SECRET_KEY`

# SD Card
* Format the SD Card as `FAT32`(recommand) or `ext4`.