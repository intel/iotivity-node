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

  // addr.adapter
  returnValue->Set(NanNew<String>("adapter"), NanNew<Number>(address->adapter));

  // addr.flags
  returnValue->Set(NanNew<String>("flags"), NanNew<Number>(address->flags));

  // addr.addr
  Local<Array> addr = NanNew<Array>(MAX_ADDR_STR_SIZE);
  for (index = 0; index < MAX_ADDR_STR_SIZE; index++) {
    addr->Set(index, NanNew<Number>(address->addr[index]));
  }
  returnValue->Set(NanNew<String>("addr"), addr);

  // addr.interface
  returnValue->Set(NanNew<String>("interface"),
                   NanNew<Number>(address->interface));

  // addr.port
  returnValue->Set(NanNew<String>("port"), NanNew<Number>(address->port));

  // addr.identity
  Local<Array> identity = NanNew<Array>(address->identity.id_length);
  for (index = 0; index < address->identity.id_length; index++) {
    identity->Set(index, NanNew<Number>(address->identity.id[index]));
  }
  returnValue->Set(NanNew<String>("identity"), identity);

  return returnValue;
}

bool c_OCDevAddr(Local<Object> jsDevAddr, OCDevAddr *address) {
  uint32_t index, length;
  OCDevAddr local;

  // addr.adapter
  Local<Value> adapter = jsDevAddr->Get(NanNew<String>("adapter"));
  VALIDATE_VALUE_TYPE(adapter, IsNumber, "addr.adapter", false);
  local.adapter = (OCTransportAdapter)adapter->Uint32Value();

  // addr.flags
  Local<Value> flags = jsDevAddr->Get(NanNew<String>("flags"));
  VALIDATE_VALUE_TYPE(flags, IsNumber, "addr.flags", false);
  local.flags = (OCTransportFlags)flags->Uint32Value();

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

  // addr.interface
  Local<Value> interface = jsDevAddr->Get(NanNew<String>("interface"));
  VALIDATE_VALUE_TYPE(interface, IsNumber, "addr.interface", false);
  local.interface = interface->Uint32Value();

  // addr.port
  Local<Value> port = jsDevAddr->Get(NanNew<String>("port"));
  VALIDATE_VALUE_TYPE(port, IsNumber, "addr.port", false);
  local.port = port->Uint32Value();

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
