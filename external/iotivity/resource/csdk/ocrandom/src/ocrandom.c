//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


#if defined(__ANDROID__) || defined(__linux__)
#include "fcntl.h"
#include "unistd.h"
#endif
#include "ocrandom.h"

#ifdef ARDUINO
#include "Arduino.h"

uint8_t GetRandomBitRaw() {
    return analogRead((uint8_t)ANALOG_IN) & 0x1;
}

uint8_t GetRandomBitRaw2() {
    int a = 0;
    for(;;) {
        a = GetRandomBitRaw() | (GetRandomBitRaw()<<1);
        if (a==1){
            return 0; // 1 to 0 transition: log a zero bit
        }
        if (a==2){
            return 1;// 0 to 1 transition: log a one bit
        }
        // For other cases, try again.
    }
}

uint8_t GetRandomBit() {
    int a = 0;
    for(;;) {
        a = GetRandomBitRaw2() | (GetRandomBitRaw2()<<1);
        if (a==1){
            return 0; // 1 to 0 transition: log a zero bit
        }
        if (a==2){
            return 1;// 0 to 1 transition: log a one bit
        }
        // For other cases, try again.
    }
}
#endif

int8_t OCSeedRandom() {
#if defined(__ANDROID__) || defined(__linux__)
    int32_t fd = open("/dev/urandom", O_RDONLY);
    if (fd > 0) {
        uint32_t randomSeed;
        uint32_t totalRead = 0; //how many integers were read
        int32_t currentRead = 0;
        while (totalRead < sizeof(randomSeed)) {
            currentRead = read(fd, (uint8_t*) &randomSeed + totalRead,
                    sizeof(randomSeed) - totalRead);
            if(currentRead > 0){
                totalRead += currentRead;
            }
        }
        close(fd);
        srand(randomSeed);
        return 0;
    }
    close(fd);
    return -1;
#elif defined ARDUINO
    uint32_t result =0;
    uint8_t i;
    for (i=32; i--;){
        result += result + GetRandomBit();
    }
    randomSeed(result);
    return 0;
#endif

}

void OCFillRandomMem(uint8_t * location, uint16_t len) {
    if(!location){
        return;
    }
    for (; len--;){
        *location++ = OCGetRandomByte();
    }
}

uint32_t OCGetRandom() {
    uint32_t result = 0;
    OCFillRandomMem((uint8_t*) &result, 4);
    return result;
}

uint8_t OCGetRandomByte(void) {
#if defined(__ANDROID__) || defined(__linux__)
    return rand() & 0x00FF;
#elif defined ARDUINO
    return random(256) & 0x00FF;
#endif
}

uint32_t OCGetRandomRange(uint32_t firstBound, uint32_t secondBound){
    uint32_t base;
    uint32_t diff;
    uint32_t result;
    if(firstBound > secondBound){
        base = secondBound;
        diff = firstBound - secondBound;
    }else if(firstBound < secondBound){
        base = firstBound;
        diff = secondBound - firstBound;
    }else{
        return secondBound;
    }
    result = ((float)OCGetRandom()/((float)(0xFFFFFFFF))*(float)diff) + (float) base;
    return result;
}
