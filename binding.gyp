{
	"targets": [
		{
			"target_name": "iotivity",
			"sources": [
				"src/main.cc",
				"src/enums.cc",
				"src/functions.cc",
				"src/callback-info.c"
			],
			"libraries": [
				"<(PRODUCT_DIR)/ocbtstack.a",
				"<(PRODUCT_DIR)/libffi.a"
			],
			"include_dirs": [
				"external/iotivity/resource/csdk/stack/include",
				"external/iotivity/resource/csdk/ocsocket/include"
			],
			"conditions": [
				[ "'<!(echo $TESTING)'=='true'",
						{ "defines": [ "TESTING" ] } ]
			],
			"dependencies": [
				"ocbtstack",
				"node_modules/ffi/deps/libffi/libffi.gyp:ffi"
			]
		},
		{
			"target_name": "ocbtstack",
			"type": "static_library",
			"defines": [
				"WITH_POSIX"
			],
			"cflags": [
				"-Os",
				"-Wall",
				"-fpic",
				"-ffunction-sections",
				"-fdata-sections",
				"-std=gnu99"
			],
			"configurations": {
				"Debug": {
					"defines!": [ "DEBUG" ]
				}
			},
			"sources": [
				"external/iotivity/resource/csdk/ocsocket/src/ocsocket.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/pdu.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/net.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/debug.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/encode.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/uri.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/coap_list.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/hashkey.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/str.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/option.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/async.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/subscribe.c",
				"external/iotivity/resource/csdk/libcoap-4.1.1/block.c",
				"external/iotivity/resource/csdk/logger/src/logger.c",
				"external/iotivity/resource/csdk/ocrandom/src/ocrandom.c",
				"external/iotivity/resource/csdk/ocmalloc/src/ocmalloc.c",
				"external/iotivity/resource/oc_logger/c/oc_logger.c",
				"external/iotivity/resource/oc_logger/c/oc_console_logger.c",
				"external/iotivity/extlibs/cjson/cJSON.c",
				"external/iotivity/resource/csdk/occoap/src/occoap.c",
				"external/iotivity/resource/csdk/occoap/src/occoaphelper.c",
				"external/iotivity/resource/csdk/stack/src/ocstack.c",
				"external/iotivity/resource/csdk/stack/src/occlientcb.c",
				"external/iotivity/resource/csdk/stack/src/ocresource.c",
				"external/iotivity/resource/csdk/stack/src/ocobserve.c",
				"external/iotivity/resource/csdk/stack/src/ocserverrequest.c",
				"external/iotivity/resource/csdk/stack/src/occollection.c",
				"external/iotivity/resource/csdk/stack/src/oicgroup.c",
				"external/iotivity/resource/csdk/stack/src/ocsecurity.c"
			],
			"include_dirs": [
				"external/iotivity/extlibs/cjson",
				"external/iotivity/resource/csdk/libcoap-4.1.1",
				"external/iotivity/resource/csdk/ocsocket/include",
				"external/iotivity/resource/csdk/logger/include",
				"external/iotivity/resource/csdk/ocrandom/include",
				"external/iotivity/resource/csdk/occoap/include",
				"external/iotivity/resource/csdk/ocmalloc/include",
				"external/iotivity/resource/csdk/stack/include",
				"external/iotivity/resource/csdk/stack/include/internal",
				"external/iotivity/resource/oc_logger/include"
			]
		}
	]
}
