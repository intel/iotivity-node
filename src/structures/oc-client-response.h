#ifndef __IOTIVITY_NODE_OCCLIENTRESPONSE_H__
#define __IOTIVITY_NODE_OCCLIENTRESPONSE_H__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Object> js_OCClientResponse(OCClientResponse *response);

#endif /* __IOTIVITY_NODE_OCCLIENTRESPONSE_H__ */
