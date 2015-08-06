#include <nan.h>
#include "oc-dev-addr.h"
#include "../common.h"

using namespace v8;

Local<Object> js_OCDevAddr(OCDevAddr *address) {
  uint32_t addressIndex;
  Local<Object> returnValue = NanNew<Object>();

  // addr.addr
  Local<Array> addrAddr = NanNew<Array>(DEV_ADDR_SIZE_MAX);
  for (addressIndex = 0; addressIndex < DEV_ADDR_SIZE_MAX; addressIndex++) {
    addrAddr->Set(addressIndex, NanNew<Number>(address->addr[addressIndex]));
  }
  returnValue->Set(NanNew<String>("addr"), addrAddr);

  return returnValue;
}

bool c_OCDevAddr(Local<Object> jsDevAddr, OCDevAddr *address) {
  uint32_t addressIndex;
  uint8_t addr[DEV_ADDR_SIZE_MAX] = {0};

  Local<Value> size = jsDevAddr->Get(NanNew<String>("size"));
  VALIDATE_VALUE_TYPE(size, IsNumber, "addr.size", false);

  Local<Value> addrValue = jsDevAddr->Get(NanNew<String>("addr"));
  VALIDATE_VALUE_TYPE(addrValue, IsArray, "addr.addr", false);
  Local<Array> addrArray = Local<Array>::Cast(addrValue);
  uint32_t addrLength = addrArray->Length();
  if (addrLength > DEV_ADDR_SIZE_MAX) {
    NanThrowRangeError(
        "OCDevAddr: Number of JS structure address bytes exceeds "
        "DEV_ADDR_SIZE_MAX");
    return false;
  }

  // Grab each address byte, making sure it's a number
  for (addressIndex = 0; addressIndex < DEV_ADDR_SIZE_MAX; addressIndex++) {
    Local<Value> addressItem = addrArray->Get(addressIndex);
    VALIDATE_VALUE_TYPE(addressItem, IsNumber, "addr.addr item", false);
    addr[addressIndex] = addressItem->Uint32Value();
  }

  return true;
}
