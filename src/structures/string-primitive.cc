/*
 * Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "string-primitive.h"
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

bool c_StringNew(Local<String> jsString, char **p_string) {
  size_t length = strlen((const char *)*(String::Utf8Value(jsString)));
  char *string = (char *)malloc(length + 1);
  if (!string) {
    Nan::ThrowError("Failed to allocate memory for C string");
    return false;
  }
  string[length] = 0;
  strcpy(string, (const char *)*(String::Utf8Value(jsString)));

  *p_string = string;
  return true;
}
