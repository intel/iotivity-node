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

#include "../common.h"
#include "../structures/oc-entity-handler-response.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
#include <ocpayload.h>
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCDoResponse) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCEntityHandlerResponse response;
  OCStackResult result;

  if (!c_OCEntityHandlerResponse(info[0]->ToObject(), &response)) {
    return;
  }

  result = OCDoResponse(&response);
  if (response.payload) {
    OCPayloadDestroy(response.payload);
  }
  info.GetReturnValue().Set(Nan::New(result));
}
