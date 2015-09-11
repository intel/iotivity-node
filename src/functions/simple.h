#ifndef __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__
#define __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__

#include <v8.h>

NAN_METHOD(bind_OCGetNumberOfResourceInterfaces);
NAN_METHOD(bind_OCGetNumberOfResources);
NAN_METHOD(bind_OCGetNumberOfResourceTypes);
NAN_METHOD(bind_OCGetResourceHandle);
NAN_METHOD(bind_OCGetResourceHandleFromCollection);
NAN_METHOD(bind_OCGetResourceInterfaceName);
NAN_METHOD(bind_OCGetResourceProperties);
NAN_METHOD(bind_OCGetResourceTypeName);
NAN_METHOD(bind_OCGetResourceUri);
NAN_METHOD(bind_OCInit);
NAN_METHOD(bind_OCProcess);
NAN_METHOD(bind_OCSetDeviceInfo);
NAN_METHOD(bind_OCSetPlatformInfo);
NAN_METHOD(bind_OCStartPresence);
NAN_METHOD(bind_OCStop);
NAN_METHOD(bind_OCStopPresence);
NAN_METHOD(bind_OCUnBindResource);

#endif /* __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__ */
