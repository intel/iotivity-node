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
1. Install Raspbian. Verify the Locale and Keyboard are setup for your location (by default UK/English and UK keyboard are selected and can cause a problem for folks using a US or other country's keyboard).  You'll also need an internet connection, so now's a good time to follow the on-screen instructions for setting up a wired or wireless network.
1. Create the workspace directory:
   ```
   cd ~
   mkdir workspace
   cd workspace
   ```
1. Connect to the Internet (via Ethernet or Wi-Fi):  (this may already be set up if you did it from the NOOBS screen)
1. Install the development environment using the following instructions:
    ```
    sudo apt-get update
    sudo apt-get install scons libtool autoconf valgrind doxygen
    sudo apt-get install libboost-dev libboost-program-options-dev libboost-thread-dev \
       uuid-dev libexpat1-dev libglib2.0-dev libsqlite3-dev libcurl4-gnutls-dev libssl-dev
    ```
    1. Install the latest nvm (version 0.33.8 as of Dec 2017)
       ```
       curl https://raw.githubusercontent.com/creationix/nvm/v0.33.8/install.sh | bash
       ```
    1. Close the terminal window and reopen to finish getting environment set up
    1. Install nodejs version 8.1.3:
        ```
        nvm install 8.1.3
        nvm ls # to verify that 8.1.3 is the default
        ```
        if nvm ls does not indicate version 8.1.3 as the default, type:
        ```
        nvm use 8.1.3
        ```
    1. Install IoTivity-node using the following instructions:
        ```
        cd ~/workspace
        git clone https://github.com/otcshare/iotivity-node
        cd iotivity-node
        npm install
        ```
    1. Install rpi-gpio using the instructions here:
        ```
        npm install rpi-gpio
        ```
    1. Install the Enviro Phat Python libraries
        ```
        sudo apt-get install pimoroni
        sudo apt-get install python3-envirophat
        sudo apt-get update
        curl https://get.pimoroni.com/envirophat | bash
        ```
 1. Test access to the EnviroPhat LEDS from the command line:
     ```
     gpio mode 7 out   # configure the LED GPIO to output mode
     gpio write 7 1    # turns on the EnviroPhat LEDs
     gpio write 7 0    # turns them off
     ```
 1. Now we can run the iotivity-node code to control the light and read the sensors
     1. Open a terminal window to run the server:
        ```
        cd ~/workspace/iotivity-node/js/envirophat
        node server.envirophat.ocf.js
        ```
     1. Open another terminal window to run the client:
        ```
        cd ~/workspace/iotivity-node/js/envirophat
        node client.envirophat.ocf.js
        ```
        * Type 0 to turn off the led
        * Type 1 to turn on the led
        * Type 2 to read the tempurature
        * Type 3 to read the accelerometer
        * Type 4 to read the light color RGB values.

# Notes and known issues
* The client response payload is one state behind. To get the current state, you can read it from the server or issue the command again on the client side to get the following state.
