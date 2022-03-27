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
#include <msp430.h>
#include <stdlib.h>
#include <time.h>
#include "mesp-ws2812b.h"
#include "ws2812b.h"
#include "mesp.h"

static void mespWS2812B_decodeFrame(mesp_data_frame_t *frame);

static void mespWS2812B_effectNone(void);
static void mespWS2812B_effectRainbow(void);
static void mespWS2812B_effectPulse(void);
static void mespWS2812B_effectRandom(void);
static void mespWS2812B_effectGradient(void);
static void mespWS2812B_effectFire(void);
static void mespWS2812B_effectStartlight(void);
static void mespWS2812B_effectSpectrum(void);

static void_void_fct_t effect_fct;

void mespWS2812B_init(void)
{
    ws2812b_init();
    mesp_init(&mespWS2812B_decodeFrame);
    effect_fct = &mespWS2812B_effectNone;
}

inline void mespWS2812B_loop(void)
{
    mesp_loop();
    effect_fct();
}

void mespWS2812B_clear(void)
{
    effect_fct = &mespWS2812B_effectNone;
    ws2812b_clearStrip();
    ws2812b_showStrip();
}

void mespWS2812B_single(mespWS2812B_color_t *color)
{
    effect_fct = &mespWS2812B_effectNone;
    ws2812b_fillStrip(color->r, color->g, color->b);
    ws2812b_showStrip();
}

void mespWS2812B_individual(mespWS2812B_color_t *colors, uint8_t length)
{
    effect_fct = &mespWS2812B_effectNone;
    ws2812b_clearStrip();
    uint8_t i;
    for (i = 0; i < length; i++)
    {
        ws2812b_setLEDColor(i, colors[i].r, colors[i].g, colors[i].b);
    }
    ws2812b_showStrip();
}

void mespWS2812B_random(uint8_t length)
{
    srand(time(NULL));
    uint8_t i;
    for (i = 0; i < length; i++)
    {
        ws2812b_setLEDColor(i, (uint8_t) rand(), (uint8_t) rand(),
                            (uint8_t) rand());
    }
    ws2812b_showStrip();
}

inline void mespWS2812B_enable(void)
{
    mesp_enableIncoming();
    __bis_SR_register(GIE);
}

inline void mespWS2812B_disable(void)
{
    mesp_disableIncoming();
    __bic_SR_register(GIE);
}

static void mespWS2812B_decodeFrame(mesp_data_frame_t *frame)
{
    switch (frame->cmd)
    {
    case MESP_WS2812B_CMD_CLEAR:
        if (frame->length != 0)
            break;
        ws2812b_clearStrip();
        ws2812b_showStrip();
        effect_fct = &mespWS2812B_effectNone;
        break;

    case MESP_WS2812B_CMD_SINGLE:
        if (frame->length == 3)
        {
            const uint8_t r = frame->data[0];
            const uint8_t g = frame->data[1];
            const uint8_t b = frame->data[2];
            ws2812b_fillStrip(r, g, b);
            ws2812b_showStrip();
            effect_fct = &mespWS2812B_effectNone;
        }
        break;

    case MESP_WS2812B_CMD_INDIVIDUAL:
        if (frame->length == 0)
            break;
        uint8_t i;
        for (i = 0; i < frame->length / 3; i++)
        {
            const uint8_t r = frame->data[(uint8_t) (3 * i + 0)];
            const uint8_t g = frame->data[(uint8_t) (3 * i + 1)];
            const uint8_t b = frame->data[(uint8_t) (3 * i + 2)];
            ws2812b_setLEDColor(i, r, g, b);
        }
        ws2812b_showStrip();
        effect_fct = &mespWS2812B_effectNone; // set the new effect function
        break;

    case MESP_WS2812B_CMD_RAINBOW:
        // TODO: decode effect data
        // TODO: set initial conditions
        effect_fct = &mespWS2812B_effectRainbow; // set the new effect function
        break;
    case MESP_WS2812B_CMD_PULSE:
        // TODO: decode effect data
        // TODO: set initial conditions
        effect_fct = &mespWS2812B_effectPulse; // set the new effect function
        break;
    case MESP_WS2812B_CMD_RANDOM:
        // TODO: decode effect data
        // TODO: set initial conditions
        effect_fct = &mespWS2812B_effectRandom; // set the new effect function
        break;
    case MESP_WS2812B_CMD_GRADIENT:
        // TODO: decode effect data
        // TODO: set initial conditions
        effect_fct = &mespWS2812B_effectGradient; // set the new effect function
        break;
    case MESP_WS2812B_CMD_FIRE:
        // TODO: decode effect data
        // TODO: set initial conditions
        effect_fct = &mespWS2812B_effectFire;  // set the new effect function
        break;
    case MESP_WS2812B_CMD_STARLIGHT:
        // TODO: decode effect data
        // TODO: set initial conditions
        effect_fct = &mespWS2812B_effectStartlight; // set the new effect function
        break;
    case MESP_WS2812B_CMD_SPECTRUM:
        // TODO: decode effect data
        // TODO: set initial conditions
        effect_fct = &mespWS2812B_effectSpectrum; // set the new effect function
        break;
    default:
        break;
    }
}

static void mespWS2812B_effectNone(void)
{
    // Nothing to do here as there is no effect
}
static void mespWS2812B_effectRainbow(void)
{
}
static void mespWS2812B_effectPulse(void)
{
}
static void mespWS2812B_effectRandom(void)
{
}
static void mespWS2812B_effectGradient(void)
{
}
static void mespWS2812B_effectFire(void)
{
}
static void mespWS2812B_effectStartlight(void)
{
}
static void mespWS2812B_effectSpectrum(void)
{
}
