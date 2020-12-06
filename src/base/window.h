
// There's one window for now and inline functions to get access

#pragma once

#include <stdbool.h>
#include "glad/glad.h"
#define GLFW_NO_INCLUDE_API
#include <GLFW/glfw3.h>
#include "base.h"

#define MAX_TITLE_LENGTH 30

bool window_create( const int width, const int height, const char* title );

void window_delete();

extern float window_get_center_x();

extern float window_get_center_y();

extern float window_get_width();

extern float window_get_height();

extern GLFWwindow *window_get_window();

extern GLenum window_get_draw_mode();
