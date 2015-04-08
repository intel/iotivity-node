# //******************************************************************
# //
# // Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# //
# // Licensed under the Apache License, Version 2.0 (the "License");
# // you may not use this file except in compliance with the License.
# // You may obtain a copy of the License at
# //
# //      http://www.apache.org/licenses/LICENSE-2.0
# //
# // Unless required by applicable law or agreed to in writing, software
# // distributed under the License is distributed on an "AS IS" BASIS,
# // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# // See the License for the specific language governing permissions and
# // limitations under the License.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# override with `make BUILD=debug`
# default to release build
BUILD := release

LIB_DIR=./lib/
BIN_DIR=./bin/

.PHONY: c_lib cpp_lib static_libs examples samples

all: dirs c_lib cpp_lib static_libs examples samples

dirs:
	-mkdir -p $(LIB_DIR) $(BIN_DIR)

c_lib:
	cd c && $(MAKE) "BUILD=$(BUILD)"

cpp_lib:
	cd cpp && $(MAKE) "BUILD=$(BUILD)"

static_libs:
	ar -cvq $(LIB_DIR)/oc_logger_core.a $(BIN_DIR)/oc_logger.o
	ar -cvq $(LIB_DIR)/oc_logger.a $(BIN_DIR)/oc_logger.o $(BIN_DIR)/targets/*.o

examples:
	cd examples && $(MAKE) "BUILD=$(BUILD)"

samples:
	cd samples/linux && $(MAKE) "BUILD=$(BUILD)"

clean:
	cd examples && $(MAKE) "clean"
	cd samples/linux && $(MAKE) "clean"
	rm -rf $(BIN_DIR)/targets/*.o
	cd cpp && $(MAKE) clean
	cd c && $(MAKE) clean
