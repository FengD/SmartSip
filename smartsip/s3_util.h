#ifndef S3_UTIL_H_
#define S3_UTIL_H_

#include <Arduino.h>

String generate_signature(String method, String date, String object_name);
int upload_file(const char* file_path);

#endif