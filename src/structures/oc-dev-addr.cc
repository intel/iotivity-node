#include <nan.h>
#include "oc-dev-addr.h"
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

Local<Object> js_OCDevAddr(OCDevAddr *address) {
  uint32_t index;
  Local<Object> returnValue = Nan::New<Object>();

  SET_VALUE_ON_OBJECT(returnValue, Number, address, adapter);
  SET_VALUE_ON_OBJECT(returnValue, Number, address, flags);
  SET_VALUE_ON_OBJECT(returnValue, Number, address, interface);
  SET_VALUE_ON_OBJECT(returnValue, Number, address, port);

  // addr.addr
  Local<Array> addr = Nan::New<Array>(MAX_ADDR_STR_SIZE);
  for (index = 0; index < MAX_ADDR_STR_SIZE; index++) {
    Nan::Set(addr, index, Nan::New(address->addr[index]));
  }
  Nan::Set(returnValue, Nan::New("addr").ToLocalChecked(), addr);

  return returnValue;
}

bool c_OCDevAddr(Local<Object> jsDevAddr, OCDevAddr *address) {
  uint32_t index, length;
  OCDevAddr local;

  VALIDATE_AND_ASSIGN(local, adapter, OCTransportAdapter, IsUint32, "addr",
                      false, jsDevAddr, Uint32Value);
  VALIDATE_AND_ASSIGN(local, flags, OCTransportFlags, IsUint32, "addr", false,
                      jsDevAddr, Uint32Value);
  VALIDATE_AND_ASSIGN(local, interface, uint32_t, IsUint32, "addr", false,
                      jsDevAddr, Uint32Value);
  VALIDATE_AND_ASSIGN(local, port, uint16_t, IsUint32, "addr", false, jsDevAddr,
                      Uint32Value);

  // addr.addr
  Local<Value> addr =
      Nan::Get(jsDevAddr, Nan::New("addr").ToLocalChecked()).ToLocalChecked();
  VALIDATE_VALUE_TYPE(addr, IsArray, "addr.addr", false);
  Local<Array> addrArray = Local<Array>::Cast(addr);
  length = addrArray->Length();
  if (length > MAX_ADDR_STR_SIZE) {
    Nan::ThrowRangeError(
        "OCDevAddr: Number of JS structure address bytes exceeds "
        "MAX_ADDR_STR_SIZE");
    return false;
  }
  for (index = 0; index < MAX_ADDR_STR_SIZE; index++) {
    if (index < length) {
      Local<Value> addressItem = Nan::Get(addrArray, index).ToLocalChecked();
      VALIDATE_VALUE_TYPE(addressItem, IsUint32, "addr.addr item", false);
      local.addr[index] = (char)addressItem->Uint32Value();
    } else {
      local.addr[index] = 0;
    }
  }

  // We only touch the structure we're supposed to fill in if all retrieval from
  // JS has gone well
  memcpy(address, &local, sizeof(OCDevAddr));

  return true;
}
