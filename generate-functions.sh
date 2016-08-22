#!/bin/bash

# Copyright 2016 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

mkdir -p generated/

# Create the files anew
cp -f /dev/null generated/functions.cc
cp -f /dev/null generated/function-prototypes.h

echo '#ifndef __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__' >> generated/function-prototypes.h
echo '#define __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__' >> generated/function-prototypes.h
echo '' >> generated/function-prototypes.h
echo '#include <nan.h>' >> generated/function-prototypes.h
echo '' >> generated/function-prototypes.h

echo '#include "../src/functions.h"' >> generated/functions.cc
echo '#include "function-prototypes.h"' >> generated/functions.cc
echo '' >> generated/functions.cc
echo 'void InitFunctions(Handle<Object> exports, Handle<Object> module) {' >> generated/functions.cc

find src -type f | while read filename; do
	cat "${filename}" | grep NAN_METHOD;
done | \
	sed -e 's/^NAN_METHOD[[:space:]]*([[:space:]]*//' -e 's/[[:space:]]*).*$//' -e 's/^bind_//' | \
	sort -u | \
while read methodname; do
	echo "NAN_METHOD(bind_${methodname});" >> generated/function-prototypes.h
	echo "  SET_FUNCTION(exports, ${methodname});" >> generated/functions.cc
done

echo '' >> generated/function-prototypes.h
echo '#endif /* ndef __IOTIVITY_NODE_FUNCTION_PROTOTYPES_H__ */' >> generated/function-prototypes.h

echo "}" >> generated/functions.cc
