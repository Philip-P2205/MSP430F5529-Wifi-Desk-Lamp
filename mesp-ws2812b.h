/*
 * Copyright 2022 Philip Prohaska and Jonathan Margreiter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required b-y applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */
#ifndef MESP_WS2812B_H_
#define MESP_WS2812B_H_

#include <stdint.h>

typedef void (*void_void_fct_t)(void);

typedef struct
{
    uint8_t r, g, b;
} mespWS2812B_color_t;

extern void mespWS2812B_init(void);
extern inline void mespWS2812B_loop(void);

extern inline void mespWS2812B_enable(void);
extern inline void mespWS2812B_disable(void);

/**
 * Clears the led strip
 */
extern void mespWS2812B_clear(void);
/**
 * sets the led strip to a single color
 */
extern void mespWS2812B_single(mespWS2812B_color_t *color);
extern void mespWS2812B_individual(mespWS2812B_color_t *colors, uint8_t length);
extern void mespWS2812B_rainbow(void);
extern void mespWS2812B_pulse(mespWS2812B_color_t *color);
extern void mespWS2812B_random(uint8_t length);
extern void mespWS2812B_gradient(mespWS2812B_color_t *color1,
                                 mespWS2812B_color_t *color2);
extern void mespWS2812B_fire(void);
extern void mespWS2812B_starlight(void);
extern void mespWS2812B_spectrum(void);

#define MESP_WS2812B_CMD_CLEAR 0x01
#define MESP_WS2812B_CMD_SINGLE 0x02
#define MESP_WS2812B_CMD_INDIVIDUAL 0x03
#define MESP_WS2812B_CMD_RAINBOW 0x04
#define MESP_WS2812B_CMD_PULSE 0x05
#define MESP_WS2812B_CMD_RANDOM 0x06
#define MESP_WS2812B_CMD_GRADIENT 0x07
#define MESP_WS2812B_CMD_FIRE 0x08
#define MESP_WS2812B_CMD_STARLIGHT 0x09
#define MESP_WS2812B_CMD_SPECTRUM 0x0A

#endif /* MESP_WS2812B_H_ */
