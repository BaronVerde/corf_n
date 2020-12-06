
#pragma once

#include "omath/vec4.h"

typedef vec4f color_t;

static const color_t color_cornflower_blue = { 0.394f, 0.609f, 0.933f, 1.0f };
static const color_t color_purple = { 0.625f, 0.125f, 0.937f, 1.0f };
static const color_t color_white = { 1.0f, 1.0f, 1.0f, 1.0f };
static const color_t color_pink = { 0.93f, 0.51f, 0.93f, 1.0f };
static const color_t color_black = { 0.0f, 0.0f, 0.0f, 1.0f };
static const color_t color_light_slate_gray = { 0.464f, 0.531f, 0.597f, 1.0f };
static const color_t color_gray = { 0.742f, 0.742f, 0.742f, 1.0f };

// Rainbow colors to visualize selected level boxes
static const color_t color_red = { 1.0f, 0.0f, 0.0f, 1.0f };
static const color_t color_orange = { 1.0f, 0.5f, 0.0f, 1.0f };
static const color_t color_yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
static const color_t color_green = { 0.0f, 1.0f, 0.0f, 1.0f };
static const color_t color_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
static const color_t color_indigo = { 0.29f, 0.0f, 0.51f, 1.0f };
static const color_t color_violet = { 0.58f, 0.0f, 0.83f, 1.0f };

static const color_t color_rainbow[7] = {
		color_violet, color_indigo, color_blue, color_green, color_yellow, color_orange, color_red
};
