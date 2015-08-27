#include <nan.h>
#include "oc-dev-addr.h"
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

Local<Object> js_OCDevAddr(OCDevAddr *address) {
  uint32_t index;
  Local<Object> returnValue = NanNew<Object>();

  SET_VALUE_ON_OBJECT(returnValue, Number, address, adapter);
  SET_VALUE_ON_OBJECT(returnValue, Number, address, flags);
  SET_VALUE_ON_OBJECT(returnValue, Number, address, interface);
  SET_VALUE_ON_OBJECT(returnValue, Number, address, port);

  // addr.addr
  Local<Array> addr = NanNew<Array>(MAX_ADDR_STR_SIZE);
  for (index = 0; index < MAX_ADDR_STR_SIZE; index++) {
    addr->Set(index, NanNew<Number>(address->addr[index]));
  }
  returnValue->Set(NanNew<String>("addr"), addr);

  // addr.identity
  uint16_t identityLength = (address->identity.id_length > MAX_IDENTITY_SIZE
                                 ? MAX_IDENTITY_SIZE
                                 : address->identity.id_length);
  Local<Array> identity = NanNew<Array>(identityLength);
  for (index = 0; index < identityLength; index++) {
    identity->Set(index, NanNew<Number>(address->identity.id[index]));
  }
  returnValue->Set(NanNew<String>("identity"), identity);

  return returnValue;
}

bool c_OCDevAddr(Local<Object> jsDevAddr, OCDevAddr *address) {
  uint32_t index, length;
  OCDevAddr local;

  VALIDATE_AND_ASSIGN(local, adapter, OCTransportAdapter, IsNumber, "addr",
                      false, jsDevAddr, Uint32Value);
  VALIDATE_AND_ASSIGN(local, flags, OCTransportFlags, IsNumber, "addr", false,
                      jsDevAddr, Uint32Value);
  VALIDATE_AND_ASSIGN(local, interface, uint32_t, IsNumber, "addr", false,
                      jsDevAddr, Uint32Value);
  VALIDATE_AND_ASSIGN(local, port, uint16_t, IsNumber, "addr", false, jsDevAddr,
                      Uint32Value);

  // addr.addr
  Local<Value> addr = jsDevAddr->Get(NanNew<String>("addr"));
  VALIDATE_VALUE_TYPE(addr, IsArray, "addr.addr", false);
  Local<Array> addrArray = Local<Array>::Cast(addr);
  length = addrArray->Length();
  if (length > MAX_ADDR_STR_SIZE) {
    NanThrowRangeError(
        "OCDevAddr: Number of JS structure address bytes exceeds "
        "MAX_ADDR_STR_SIZE");
    return false;
  }
  for (index = 0; index < MAX_ADDR_STR_SIZE; index++) {
    if (index < length) {
      Local<Value> addressItem = addrArray->Get(index);
      VALIDATE_VALUE_TYPE(addressItem, IsNumber, "addr.addr item", false);
      local.addr[index] = (char)addressItem->Uint32Value();
    } else {
      local.addr[index] = 0;
    }
  }

  // addr.identity
  Local<Value> identity = jsDevAddr->Get(NanNew<String>("identity"));
  VALIDATE_VALUE_TYPE(identity, IsArray, "addr.identity", false);
  Local<Array> identityArray = Local<Array>::Cast(identity);
  length = identityArray->Length();
  if (length > MAX_IDENTITY_SIZE) {
    NanThrowRangeError(
        "OCDevAddr: Number of JS structure identity bytes exceeds "
        "MAX_IDENTITY_SIZE");
    return false;
  }
  for (index = 0; index < MAX_IDENTITY_SIZE; index++) {
    if (index < length) {
      Local<Value> identityItem = identityArray->Get(index);
      VALIDATE_VALUE_TYPE(identityItem, IsNumber, "addr.identity.id item",
                          false);
      local.identity.id[index] = (unsigned char)identityItem->Uint32Value();
    } else {
      local.identity.id[index] = 0;
    }
  }
  local.identity.id_length = length;

  // We only touch the structure we're supposed to fill in if all retrieval from
  // JS has gone well
  memcpy(address, &local, sizeof(OCDevAddr));

  return true;
}
