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

#include <nan.h>

#include "constants.h"
#include "enums.h"
#include "functions.h"

using namespace v8;

#ifdef DEBUG
#include <stdarg.h>
Local<Value> json_stringify(Local<Object> jsObject) {
  Local<Value> arguments[3] = {jsObject, Nan::Null(), Nan::New(4)};
  return Nan::MakeCallback(
      Nan::GetCurrentContext()->Global(),
      Local<Function>::Cast(
          Nan::Get(Nan::To<Object>(Nan::Get(Nan::GetCurrentContext()->Global(),
                                            Nan::New("JSON").ToLocalChecked())
                                       .ToLocalChecked())
                       .ToLocalChecked(),
                   Nan::New("stringify").ToLocalChecked())
              .ToLocalChecked()),
      3, arguments);
}
void console_log(Local<Value> argument) {
  String::Utf8Value something(argument);
  Nan::MakeCallback(
      Nan::GetCurrentContext()->Global(),
      Local<Function>::Cast(
          Nan::Get(
              Nan::To<Object>(Nan::Get(Nan::GetCurrentContext()->Global(),
                                       Nan::New("console").ToLocalChecked())
                                  .ToLocalChecked())
                  .ToLocalChecked(),
              Nan::New("log").ToLocalChecked())
              .ToLocalChecked()),
      1, &argument);
}
void debug_print(const char *message, ...) {
  const char *format = "{\"info\":true,\"message\":\"** %s\"}";
  char *buffer1 = 0, *buffer2 = 0;
  size_t length;
  va_list va;

  va_start(va, message);
  length = vsnprintf(0, 0, message, va);
  va_end(va);

  buffer1 = (char *)malloc(length + 1);

  va_start(va, message);
  vsnprintf(buffer1, length + 1, message, va);
  va_end(va);

  length = snprintf(0, 0, format, buffer1);
  buffer2 = (char *)malloc(length + 1);
  snprintf(buffer2, length + 1, format, buffer1);

  free(buffer1);

  console_log(Nan::New(buffer2).ToLocalChecked());
  free(buffer2);
}
#endif /* def DEBUG */

NAN_MODULE_INIT(Init) {
  InitFunctions(target);
  InitEnums(target);
  InitConstants(target);
}

NODE_MODULE(iotivity, Init)
