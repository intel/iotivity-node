#ifndef __IOTIVITY_NODE_OCDEVADDR_H__
#define __IOTIVITY_NODE_OCDEVADDR_H__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Object> js_OCDevAddr(OCDevAddr *address);

// Fills in @address
bool c_OCDevAddr(v8::Local<v8::Object> jsDevAddr, OCDevAddr *address);
v8::Local<v8::Object> js_OCDevAddr( OCDevAddr *address );

#endif /* __IOTIVITY_NODE_OCDEVADDR_H__ */
