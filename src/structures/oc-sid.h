#ifndef __IOTIVITY_NODE_OCSID_H__
#define __IOTIVITY_NODE_OCSID_H__

#include <v8.h>

void fillJSSid(v8::Local<v8::Array> jsSid, uint8_t *sid);
v8::Local<v8::Array> js_SID(uint8_t *sid);
bool c_SID(v8::Local<v8::Array> jsSid, uint8_t *sid);

#endif /* __IOTIVITY_NODE_OCSID_H__ */
