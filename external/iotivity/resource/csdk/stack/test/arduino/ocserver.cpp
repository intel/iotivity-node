#include <Time.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ocstack.h>
#include <logger.h>

static uint8_t ETHERNET_MAC[] = {0x90, 0xA2, 0xDA, 0x0F, 0x2B, 0x72 };

#define TAG PCF("ocserver")

void ocInitialize () {
    char ipAddr[16] = "";
    OCGetInterfaceAddress (NULL, 0, AF_INET, (uint8_t *)ipAddr, 16);
    OC_LOG(DEBUG, TAG, PCF("IP addr is:"));
    OC_LOG_BUFFER(INFO, TAG, (uint8_t*)ipAddr, sizeof(ipAddr));
    delay(2000);
    OCInit (ipAddr, 8001, OC_SERVER);
}

void setup() {
	Serial.begin(115200);
   
    Serial.println ("Trying to get an IP addr using DHCP");
    if (Ethernet.begin(ETHERNET_MAC) == 0) {
        Serial.println("DHCP failed");
    }
	ocInitialize ();
}

void loop() {
	Serial.println ("Processing CoAP messages!\n");
    OCProcess ();
	delay(1000);
}
