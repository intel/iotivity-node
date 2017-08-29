''' Copyright 2017 Vprime
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
# limitations under the License.*/

 @author Rami Alshafi <ralshafi@vprime.com> '''
# to install the envirophat dependecies, type the following command 
# $ curl https://get.pimoroni.com/envirophat | bash
# make sure to type 'y' or 'n' when prompted
import sys
import envirophat

if __name__ == "__main__":
	arguments = sys.argv
	if len(arguments) == 2:
		if arguments[1] == "0":
			envirophat.leds.off()
			print "leds:status:False"
		elif arguments[1] == "1":
			envirophat.leds.on()
			print "leds:status:True"
		elif arguments[1] == "2":
			print "weather:temperature:"+str(envirophat.weather.temperature())
		elif arguments[1] == "3":
			print "motion:accelerometer:"+str(list(envirophat.motion.accelerometer()))
		elif arguments[1] == "4":
			print "light:rgb:"+str(list(envirophat.light.rgb()))
	else:
		print "error:wrong_command:please press any of the following options: 0, 1, 2, 3, 4"
