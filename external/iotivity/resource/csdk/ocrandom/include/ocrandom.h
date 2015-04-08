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


#ifndef _RANDOM_H
#define _RANDOM_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__ANDROID__) || defined(__linux__)
#include <time.h>
#elif defined ARDUINO
// MEGA has 16 input pins whereas Due has only 12 input pins
#define ANALOG_IN (10)
#endif

/**
 * Seed the random number generator. Seeding depends on platform.
 * Android and Linux uses current time. Arduino uses Analog reading on pin ANALOG_IN
 * @retval 0 for Success, otherwise some error value
 */
int8_t OCSeedRandom();

/**
 * Generate a uniformly [0,2^32] distributed random number
 * @retval On Success, it returns the random value.
 */
uint32_t OCGetRandom();

/**
 * Generate a uniformly [0,2^8] distributed random number
 * @retval On Success, it returns the random value, otherwise -1 for error.
 */
uint8_t OCGetRandomByte(void);

/**
 * Generate a uniformly distributed 8-bit (byte) array random numbers
 * @param[out] location
 *              memory location to start filling with random bytes
 * @param[in] len
 *              length of array to be filled with random bytes
 */
void OCFillRandomMem(uint8_t * location, uint16_t len);

/*
 * Generate a uniformly distributed number on the defined bounded range
 * @param[in] firstBound
 *              the first bound of the range
 * @param[in] secondBound
 *              the second bound of the range
 */
uint32_t OCGetRandomRange(uint32_t firstBound, uint32_t secondBound);

#ifdef __cplusplus
}
#endif

#endif //_RANDOM_H
