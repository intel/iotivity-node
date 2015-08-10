{
	"variables": {
		"externalOCTBStack": '<!(if test "x${OCTBSTACK_CFLAGS}x" == "xx" -o "x${OCTBSTACK_CFLAGS}x" == "xx"; then echo false; else echo true; fi)',
	},
	"target_defaults": {
		"include_dirs": [
			"<!(node -e \"require('nan')\")"
		],
		"conditions": [
			[ "'<(externalOCTBStack)'=='false'", {
				"libraries": [
					'<!@(echo "-L$(pwd)/deps/iotivity/lib -loctbstack -Wl,-rpath $(pwd)/deps/iotivity/lib")'
				],
				"cflags": [
					'<!@(echo "-I$(pwd)/deps/iotivity/include/iotivity/resource/csdk/stack/include")'
				],
				"xcode_settings": {
					"OTHER_CFLAGS": [
						'<!@(echo "-I$(pwd)/deps/iotivity/include/iotivity/resource/csdk/stack/include")'
					]
				}
			} ],
			[ "'<(externalOCTBStack)'=='true'", {
				"libraries": [ '<!@(echo "$OCTBSTACK_LIBS")' ],
				"cflags": [ '<!@(echo "$OCTBSTACK_CFLAGS")' ],
				"xcode_settings": {
					"OTHER_CFLAGS": [ '<!@(echo "$OCTBSTACK_CFLAGS")' ]
				}
			} ]
		],
		"cflags_cc": [ '-std=c++11' ]
	},
	"targets": [
		{
			"target_name": "csdk",
			"conditions": [
				[ "'<(externalOCTBStack)'=='false'", {
					"actions": [ {
						"action_name": "build",
						"inputs": [],
						"outputs": [
							"deps/iotivity/lib/liboctbstack.so",
							"deps/iotivity/include"
						],
						"action": [
							"sh",
							"./build-csdk.sh",
							'<!@(if test "x${npm_config_debug}x" == "xtruex"; then echo "--debug"; else echo ""; fi)'
						],
						"message": "Building CSDK"
					} ]
				} ]
			]
		},
		{
			"target_name": "iotivity",
			"sources": [
				"src/main.cc",
				"src/common.cc",
				"src/enums.cc",
				"src/constants.cc",
				"src/structures.cc",
				"src/functions/oc-create-delete-resource.cc",
				"src/functions/oc-do-resource.cc",
				"src/functions/oc-do-response.cc",
				"src/functions/oc-notify.cc",
				"src/functions/simple.cc",
				"src/functions.cc",
				"src/callback-info.c"
			],
			"libraries": [ "<(PRODUCT_DIR)/libffi.a" ],
			"conditions": [
				[ "'<!(echo $TESTING)'=='true'",
						{ "defines": [ "TESTING" ] } ]
			],
			"dependencies": [
				"csdk",
				"node_modules/ffi/deps/libffi/libffi.gyp:ffi"
			]
		},
		{
			"target_name": "client",
			"dependencies": [ "csdk" ],
			"type": "executable",
			"sources": [ "c/client.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		},
		{
			"target_name": "client.observe",
			"dependencies": [ "csdk" ],
			"type": "executable",
			"sources": [ "c/client.observe.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		},
		{
			"target_name": "server",
			"dependencies": [ "csdk" ],
			"type": "executable",
			"sources": [ "c/server.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		},
		{
			"target_name": "server.observable",
			"dependencies": [ "csdk" ],
			"type": "executable",
			"sources": [ "c/server.observable.c" ],
			"libraries": [ '<!(pkg-config --libs glib-2.0)' ],
			"cflags": [ '<!(pkg-config --cflags glib-2.0)' ],
			"xcode_settings": {
				"OTHER_CFLAGS": ['<!(pkg-config --cflags glib-2.0)'],
				"OTHER_LDFLAGS": [
					'-loctbstack', '-lconnectivity_abstraction', '-lc_common', '-lcoap'
				]
			}
		}
	]
}
