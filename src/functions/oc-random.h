#ifndef __IOTIVITY_NODE_FUNCTIONS_OC_RANDOM_H__
#define __IOTIVITY_NODE_FUNCTIONS_OC_RANDOM_H__

#include <v8.h>

NAN_METHOD(bind_OCConvertUuidToString);
NAN_METHOD(bind_OCFillRandomMem);
NAN_METHOD(bind_OCGenerateUuid);
NAN_METHOD(bind_OCGenerateUuidString);
NAN_METHOD(bind_OCGetRandom);
NAN_METHOD(bind_OCGetRandomByte);
NAN_METHOD(bind_OCGetRandomRange);
NAN_METHOD(bind_OCSeedRandom);

#endif /* __IOTIVITY_NODE_FUNCTIONS_OC_RANDOM_H__ */
