#ifndef __IOTIVITY_NODE_OCPAYLOAD_H__
#define __IOTIVITY_NODE_OCPAYLOAD_H__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Value> js_OCPayload( OCPayload *payload );

#endif /* __IOTIVITY_NODE_OCCLIENTRESPONSE_H__ */
