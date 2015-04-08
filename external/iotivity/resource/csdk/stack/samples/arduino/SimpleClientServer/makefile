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
BUILD := release
ARDUINO_PORT := /dev/ttyACM0
APP_INSTALL := ocserver
PLATFORM := arduinomega
ARDUINOWIFI := 0

ocserver:
	$(MAKE) -c ./$@ "BUILD=$(BUILD)" "install" "PLATFORM=$(PLATFORM)" "ARDUINO_PORT=$(ARDUINO_PORT)" "BUILD=$(BUILD)" "ARDUINOWIFI=$(ARDUINOWIFI)"

occlient:
	$(MAKE) -c ./$@ "BUILD=$(BUILD)"

install:
	$(MAKE) -C ./$(APP_INSTALL) "install" "PLATFORM=$(PLATFORM)" "ARDUINO_PORT=$(ARDUINO_PORT)" "BUILD=$(BUILD)" "ARDUINOWIFI=$(ARDUINOWIFI)"

clean:
	$(MAKE) -C ./occlient "clean"
	$(MAKE) -C ./ocserver "clean"
