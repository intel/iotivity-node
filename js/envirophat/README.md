# Parts list:
* Raspberry Pi Zero W  (https://www.adafruit.com/product/3410) $34.5
* You will need a USB powered hub, USB keyboard, USB mouse, HDMI cable and a Monitor with HDMI
* Pimoroni Enviro Phat sensor and actuator board (https://www.adafruit.com/product/3194) $19.95
# Hardware Instructions:
* Solder the headers
* Connect the Enviro Phat to the GPIO header on the Raspberry Pi
* Connect the keyboard, mouse and monitor to the Raspberry Pi and insert the SD card with NOOBS installed on it (you can download noobs from (https://downloads.raspberrypi.org/NOOBS_latest)
* Connect the microUSB power supply to the microUSB connector closest to the corner of the board
# Software Installation Instructions:
1. Install Raspbian.
1. create the workspace directory
    1. cd ~
    1. mkdir workspace
    1. cd workspace
1. Connect to the Internet (via Ethernet or Wi-Fi)
1. Install the development environment using the following instructions:
    1. sudo apt-get install scons libtool autoconf valgrind doxygen
    1. sudo apt-get install libboost-dev libboost-program-options-dev libboost-thread-dev uuid-dev libexpat1-dev libglib2.0-dev libsqlite3-dev libcurl4-gnutls-dev libssl-dev
    1. install nvm
        1. sudo apt-get update
        1. curl https://raw.githubusercontent.com/creationix/nvm/v0.33.2/install.sh | bash (or whatever version is latest)
        1. close the terminal window and reopen
    1. Use nodejs version 8.1.3:
        1. nvm install 8.1.3
        1. nvm ls (to verify that 8.1.3 is the default)
            1. if nvm ls does not indicate version 8.1.3 as the default, type: nvm use 8.1.3
    1. Install IoTivity-node using the following instructions:
        1. cd ~/workspace
        1. git clone https://github.com/otcshare/iotivity-node
        1. cd iotivity-node
        1. npm install
    1. Install rpi-gpio using the instructions here:
        1. npm install rpi-gpio
    1. Install the Enviro Phat Python libraries
        1. sudo apt-get install pimoroni
        1. sudo apt-get install python3-envirophat
        1. sudo apt-get update
        1. curl https://get.pimoroni.com/envirophat | bash
    1. Set the gpio pin to output mode and test from the command line
        1. gpio mode 7 out (this can probably be put in the server program)
        1. to test: gpio write 7 1
        1. to test: gpio write 7 0
        1. The above commands should switch the lights on the Enviro Phat on and off
    1. Run the iotivity-node code to control the light and read the sensors
        1. open the server terminal window (to run the server)
            1. cd ~/workspace/iotivity-node/js
            1. node server.envirophat.ocf.js
        1. open the client terminal window (to run the client)
            1. cd ~/workspace/iotivity-node/js
            1. node client.envirophat.ocf.js
                * Type 0 to turn off the led
                * Type 1 to turn on the led
                * Type 2 to read the tempurature
                * Type 3 to read the accelerometer
                * Type 4 to read the light color RGB values.
# Notes and known issues
* The client response payload is one state behind. To get the current state, you can read it from the server or issue the command again on the client side to get the following state.
