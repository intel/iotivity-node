#ifndef __IOTIVITY_NODE_OCENTITYHANDLERRESPONSE_H__
#define __IOTIVITY_NODE_OCENTITYHANDLERRESPONSE_H__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

bool c_OCEntityHandlerResponse( v8::Local<v8::Object>, OCEntityHandlerResponse *p_response );

#endif /* __IOTIVITY_NODE_OCENTITYHANDLERRESPONSE_H__ */
