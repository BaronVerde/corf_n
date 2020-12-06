
#include <stdio.h>
#include <stdlib.h>
#include "base/logbook.h"
#include "gui/gui_window.h"
#include "base/window.h"
#include "base/camera.h"
#include "terrain/terrain.h"
#include "mesh_test/mesh_test.h"
#include "texture_test/texture_test.h"

const int window_width = 1800;
const int window_height = 1000;
float g_framerate = 0.0f;
double g_deltatime = 0.0;
gui_window_t *g_gui_window = NULL;
font_info_t *g_font_info = NULL;

bool base_setup() {
	logbook_init();
	window_create( window_width, window_height, "Testwindow" );
	vec3f position = { 0.0f, 0.0f, -5.0f };
	vec3f target = { 0.0f, 0.0f, 0.0f };
	camera_create( &position, &target );
	return true;
}

void base_cleanup() {
	window_delete();
	logbook_de_init();
}

void gui_setup() {
	const GLsizei font_height = 13;
	g_font_info = font_create( "resources/fonts/mplus-1c-bold.ttf", font_height );
	g_gui_window = gui_window_create(
			"Window data", g_font_info, 3.0, window_height-3,
			(float)window_width, (float)window_height, 250, (font_height+1)*4
	);
	gui_window_begin( g_gui_window );
		gui_window_add_static_text( g_gui_window, "Framerate:", 1.0f, (float)font_height + 1.0f );
		gui_window_add_variable( g_gui_window, gui_float, &g_framerate, 80.0f, (float)font_height + 1.0f );
		gui_window_add_static_text( g_gui_window, "<f> render mode, <v> vsync", 1.0f, (float)(font_height+1)*2.0f );
		gui_window_add_static_text( g_gui_window, "<p> cam pos <left alt> switch cursor", 1.0f, (float)(font_height+1)*3.0f );
		gui_window_add_static_text( g_gui_window, "<m> view frustum", 1.0f, (float)(font_height+1)*4.0f );
	gui_window_end( g_gui_window );
}

void gui_cleanup() {
	font_delete( g_font_info );
	gui_window_delete( g_gui_window );
}

bool scene_setup() {
	if( terrain_create( false ) ) {
		if( !terrain_setup() )
			return false;
		return true;
	}
	/*if( mesh_test_create() )
		return true;
	if( texture_test_create() )
		return true;*/
	return false;
}

void scene_cleanup() {
	terrain_delete();
	//mesh_test_delete();
	//texture_test_delete();
}

// @toggle ui display, cleanup ui code
void main_loop() {
	logbook_log( LOG_INFO, "Starting mainloop ..." );
	const vec3f gui_color = { 1.0f, 1.0f, 1.0f };
	const bool draw_boxes = false, draw_terrain = true;
	double last_frame = 0.01;
	while( !glfwWindowShouldClose( window_get_window() ) ) {
		double current_frame = glfwGetTime();
		g_deltatime = current_frame - last_frame;
		g_framerate = 1.0f / (float)( current_frame - last_frame );
		last_frame = current_frame;
		//glUseProgram( 0 );
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		camera_update_moving( (float)g_deltatime );

		// scene render here
		terrain_render( draw_boxes, draw_terrain );
		//mesh_test_render();
		//texture_test_render();

		gui_window_update( g_gui_window );
		gui_window_render( g_gui_window, &gui_color );
		glfwPollEvents();
		glfwSwapBuffers( window_get_window() );
	}
	logbook_log( LOG_INFO, "... mainloop ending" );
}

int main() {
	puts( "Program starting ..." );
	if( !base_setup() ) {
		logbook_log( LOG_ERROR, "Initialisation failed" );
		return EXIT_FAILURE;
	}

	gui_setup();
	if( scene_setup() )
		main_loop();
	scene_cleanup();
	gui_cleanup();

	base_cleanup();
	puts( "... program ending" );
	return EXIT_SUCCESS;
}
