#include "common.h"

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

using namespace v8;

void addStringArray(Local<Object> destination, OCStringLL *source,
                    const char *name) {
  if (source) {
    int counter;
    OCStringLL *item;
    for (item = source, counter = 0; item; item = item->next, counter++)
      ;
    Local<Array> jsArray = Nan::New<Array>(counter);
    for (item = source, counter = 0; item; item = item->next, counter++) {
      Nan::Set(jsArray, counter, Nan::New(item->value).ToLocalChecked());
    }
    Nan::Set(destination, Nan::New(name).ToLocalChecked(), jsArray);
  }
}
