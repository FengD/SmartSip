#include "s3_util.h"
#include <mbedtls/md.h>
#include <mbedtls/base64.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <SD.h>
#include "config.h"

String get_date_rf2616_format() {
  time_t now;
  struct tm timeinfo;
  char buf[64];

  time(&now);
  gmtime_r(&now, &timeinfo);
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
  
  return String(buf);
}

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