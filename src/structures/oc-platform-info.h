#ifndef __IOTIVITY_NODE_OCPLATFORMINFO_H__
#define __IOTIVITY_NODE_OCPLATFORMINFO_H__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Object> js_OCPlatformInfo(OCPlatformInfo *platformInfo);
bool c_OCPlatformInfo(v8::Local<v8::Object> platformInfo, OCPlatformInfo *info);
void c_OCPlatformInfoFreeMembers(OCPlatformInfo *info);

#endif /* __IOTIVITY_NODE_OCPLATFORMINFO_H__ */
