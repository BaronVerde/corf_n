
#pragma once

#include <stdbool.h>
#include "glad/glad.h"
#define GLFW_NO_INCLUDE_API
#include <GLFW/glfw3.h>
#include "omath/mat4.h"
#include "omath/vec3.h"
#include "omath/frustum.h"

#define WINDOW_MAX_TITLE_LENGTH 30

void window_create(
		const char* title, const int width, const int height,
		const vec3f *const pos, const vec3f *const target );

void window_delete();

void window_update_moving( float deltatime );

void window_set_position_and_target( const vec3f* const pos, const vec3f* const target );

void window_print_camera_position();

extern GLFWwindow *window_get_window();

extern vec3f *window_get_camera_position();

extern mat4f *window_get_view_matrix();

extern mat4f *window_get_view_perspective_matrix();
