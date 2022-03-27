/*
 * Copyright 2022 Philip Prohaska and Jonathan Margreiter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at*
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */

#ifndef MESP_H_
#define MESP_H_

#include <msp430.h>
#include <stdint.h>

#define MESP_START_CODE 0xAA
#define MESP_END_CODE 0x33

#define MESP_STATUS_START 0x01
#define MESP_STATUS_CMD 0x02
#define MESP_STATUS_LENGTH 0x03
#define MESP_STATUS_DATA 0x04
#define MESP_STATUS_END 0x05
#define MESP_STATUS_FINISHED 0x06

typedef struct
{
    uint8_t cmd;
    uint8_t length;
    uint8_t *data;
} mesp_data_frame_t;

typedef void (*mesp_callback_fct_t)(mesp_data_frame_t*);

extern void mesp_init(mesp_callback_fct_t callback);
extern void mesp_initSPI(void);
extern void mesp_loop(void);

extern inline void mesp_disableIncoming(void);
extern inline void mesp_enableIncoming(void);
#endif /* MESP_H_ */
