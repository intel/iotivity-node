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

#include "oc-dev-addr.h"
#include <nan.h>
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

Local<Object> js_OCDevAddr(OCDevAddr *address) {
  Local<Object> returnValue = Nan::New<Object>();

  SET_VALUE_ON_OBJECT(returnValue, address, adapter, Number);
  SET_VALUE_ON_OBJECT(returnValue, address, flags, Number);
  SET_VALUE_ON_OBJECT(returnValue, address, ifindex, Number);
  SET_VALUE_ON_OBJECT(returnValue, address, port, Number);
  Nan::Set(returnValue, Nan::New("addr").ToLocalChecked(),
           Nan::New(address->addr).ToLocalChecked());
  return returnValue;
}

bool c_OCDevAddr(Local<Object> jsDevAddr, OCDevAddr *address) {
  uint32_t length;
  OCDevAddr local;

  memset(&local, 0, sizeof(OCDevAddr));

  VALIDATE_AND_ASSIGN(local, jsDevAddr, adapter, OCTransportAdapter, IsUint32,
                      "addr", uint32_t, return false);
  VALIDATE_AND_ASSIGN(local, jsDevAddr, flags, OCTransportFlags, IsUint32,
                      "addr", uint32_t, return false);
  VALIDATE_AND_ASSIGN(local, jsDevAddr, ifindex, uint32_t, IsUint32, "addr",
                      uint32_t, return false);
  VALIDATE_AND_ASSIGN(local, jsDevAddr, port, uint16_t, IsUint32, "addr",
                      uint32_t, return false);

  Local<Value> addressValue =
      Nan::Get(jsDevAddr, Nan::New("addr").ToLocalChecked()).ToLocalChecked();
  VALIDATE_VALUE_TYPE(addressValue, IsString, "addr", return false);
  String::Utf8Value addressString(addressValue);
  length = strlen(*addressString);
  if (length >= MAX_ADDR_STR_SIZE) {
    Nan::ThrowError("UTF-8 representation of address string is too long");
    return false;
  }
  strcpy(local.addr, *addressString);

  // We only touch the structure we're supposed to fill in if all retrieval from
  // JS has gone well
  *address = local;
  return true;
}
