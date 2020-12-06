
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window.h"
#include "logbook.h"
#include "camera.h"
#include <stdio.h>

static void error_callback( int error, const char *msg );
static void APIENTRY glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *message, const void *userParam );
static void window_toggle_input_mode();
static void window_key_callback( GLFWwindow *window, int key, int scancode, int action, int mods );
static void window_cursor_pos_callback( GLFWwindow *window, double xPos, double yPos );

static struct gl_window {
	int width;
	int height;
	float center_x;
	float center_y;
	bool vsync;
	GLenum draw_mode;
	bool cursor_disabled;
	bool debug_enabled;
	char title[MAX_TITLE_LENGTH];
	GLFWwindow *window;
} gl_window;

bool window_create( const int width, const int height, const char* title ) {
	gl_window.window = NULL;
	gl_window.debug_enabled = true;
	gl_window.width = width;
	gl_window.height = height;
	gl_window.center_x = (float)gl_window.width/2.0f;
	gl_window.center_y = (float)gl_window.height/2.0f;
	strncpy( gl_window.title, title, MAX_TITLE_LENGTH-1 );
	gl_window.cursor_disabled = false;
	gl_window.vsync = true;
	gl_window.draw_mode = GL_TRIANGLES;
	glfwSetErrorCallback( error_callback );
	if( GL_TRUE != glfwInit() ) {
		logbook_log( LOG_ERROR, "Error initialising glfw" );
		return false;
	}
	// Window and context; we want OpenGL 4.5
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
	if( gl_window.debug_enabled )
		glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
	glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	//glfwWindowHint( GLFW_SAMPLES, 0 );
	//glfwWindowHint( GLFW_DEPTH_BITS, 32 );
	gl_window.window = glfwCreateWindow(
			gl_window.width, gl_window.height, gl_window.title, NULL, NULL
	);
	if( NULL == gl_window.window ) {
		logbook_log( LOG_ERROR, "Could not create glfw window." );
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent( gl_window.window );
	// Load gl function pointers
	if( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) ) {
		logbook_log( LOG_ERROR, "Error initialising glad" );
		glfwDestroyWindow( gl_window.window );
		glfwTerminate();
		return false;
	}
	glfwSwapInterval( gl_window.vsync ? 1 : 0 );
	if( gl_window.debug_enabled ) {
		// Initialize debug output
		GLint flags;
		glGetIntegerv( GL_CONTEXT_FLAGS, &flags );
		if( flags & GL_CONTEXT_FLAG_DEBUG_BIT ) {
			glEnable( GL_DEBUG_OUTPUT );
			// Envoke callback directly in case of error
			glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
			glDebugMessageCallback( glDebugOutput, NULL );
			glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
			logbook_log( LOG_INFO, "OpenGL 4.5 debug context created." );
		} else
			logbook_log( LOG_WARNING, "OpenGL 4.5 context (no debug) created." );
	}
	// Ensure we can capture key events
	glfwSetInputMode( gl_window.window, GLFW_STICKY_KEYS, GLFW_TRUE );
	glfwSetInputMode( gl_window.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
	glfwSetCursorPosCallback( gl_window.window, window_cursor_pos_callback );
	glfwSetKeyCallback( gl_window.window, window_key_callback );
	return true;
}

void window_delete() {
	glfwDestroyWindow( gl_window.window );
	gl_window.window = NULL;
	glfwTerminate();
}

inline float window_get_center_x() {
	return gl_window.center_x;
}

inline float window_get_center_y() {
	return gl_window.center_y;
}

inline float window_get_width() {
	return (float)gl_window.width;
}

inline float window_get_height() {
	return (float)gl_window.height;
}

inline GLFWwindow *window_get_window() {
	return gl_window.window;
}

inline GLenum window_get_draw_mode() {
	return gl_window.draw_mode;
}

// ---------------- statics -------------------
void error_callback( int error, const char *msg ) {
	char m[MAX_LEN_MESSAGES];
	snprintf( m, MAX_LEN_MESSAGES-1, "[%d] %s", error, msg );
	logbook_log( LOG_UNSPECIFIED, m );
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void APIENTRY glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *message, const void *userParam ) {
	// Ignore non-significant error/warning codes
	if( /*id == 131169 ||	// framebuffer storage allocation */
		id == 131185 /*||	// buffer memory usage
		id == 131218 ||	// shader being recompiled
		id == 131204 ||
		// shader compiler debug messages
		id == 6 || id == 7 || id == 8 || id == 9 || id == 10 ||
		id == 11 || id == 12 || id == 13 || id == 14*/ )
		return;
	char m[MAX_LEN_MESSAGES] = "Source: ";
    switch( source ) {
        case GL_DEBUG_SOURCE_API: strcat( m, "API" ); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:	strcat( m, "window system" ); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: strcat( m, "shader compiler" ); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: strcat( m, "third party" ); break;
        case GL_DEBUG_SOURCE_APPLICATION: strcat( m, "application" ); break;
        case GL_DEBUG_SOURCE_OTHER: strcat( m, "other" ); break;
        default: strcat( m, "unknown" );
    }
    strcat( m, "; Type: " );
    switch( type ) {
        case GL_DEBUG_TYPE_ERROR: strcat( m, "error" ); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strcat( m, "deprecated behavior" ); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: strcat( m, "undefined behavior" ); break;
        case GL_DEBUG_TYPE_PORTABILITY: strcat( m, "portability" ); break;
        case GL_DEBUG_TYPE_PERFORMANCE: strcat( m, "performance" ); break;
        case GL_DEBUG_TYPE_MARKER: strcat( m, "marker" ); break;
        case GL_DEBUG_TYPE_PUSH_GROUP: strcat( m, "push group" ); break;
        case GL_DEBUG_TYPE_POP_GROUP: strcat( m, "pop group" ); break;
        case GL_DEBUG_TYPE_OTHER: strcat( m, "other" ); break;
        default: strcat( m, "unknown" );
    }
    strcat( m, "; Severity: " );
    switch( severity ) {
        case GL_DEBUG_SEVERITY_HIGH: strcat( m, "high" ); break;
        case GL_DEBUG_SEVERITY_MEDIUM: strcat( m, "medium" ); break;
        case GL_DEBUG_SEVERITY_LOW: strcat( m, "low" ); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: strcat( m, "notification" ); break;
        default: strcat( m, "unknown" );
    }
    strcat( m, "; " );
    if( strlen( m ) + strlen( message ) >= MAX_LEN_MESSAGES )
    	logbook_log( LOG_UNSPECIFIED, "Gl debug output: message too long. Message truncated, pls split." );
    strncat( m, message, MAX_LEN_MESSAGES-1 );
    logbook_log( LOG_UNSPECIFIED, m );
    if( NULL != userParam ) {
    	logbook_log( LOG_UNSPECIFIED, "Additional userdata:" );
    	logbook_log( LOG_UNSPECIFIED, (char *)userParam );
    }
}

void window_key_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	bool handled = false;
	if( GLFW_PRESS == action || GLFW_REPEAT == action ) {
		switch( key ) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose( gl_window.window, GLFW_TRUE );
				handled = true;
				break;
			case GLFW_KEY_LEFT_ALT:
				window_toggle_input_mode();
				handled = true;
				break;
			case GLFW_KEY_V:
				gl_window.vsync = !gl_window.vsync;
				glfwSwapInterval( gl_window.vsync ? 1 : 0 );
				handled = true;
				break;
			case GLFW_KEY_F:
				gl_window.draw_mode = ( GL_TRIANGLES == gl_window.draw_mode ? GL_LINES : GL_TRIANGLES );
				handled = true;
				break;
		}
	}
	if( !handled )
		handled = camera_key_event( key, action, mods );
	if( !handled ) {
		// others ?
	}
}
#pragma GCC diagnostic pop

void window_toggle_input_mode() {
	gl_window.cursor_disabled = !gl_window.cursor_disabled;
	glfwSetInputMode( gl_window.window, GLFW_CURSOR,
			gl_window.cursor_disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL );
}

void window_cursor_pos_callback( GLFWwindow *window, double xPos, double yPos ) {
	if( !gl_window.cursor_disabled )
		return;
	// only handel mouse move when cursor disabled
	bool handled = false;
	handled = camera_mouse_move( (float)xPos, (float)yPos );
	if( !handled ) {
		// others ?
	}
	// @todo cursor should center automatically, according to glfw doc, but doesn't
	glfwSetCursorPos( window, gl_window.center_x, gl_window.center_y );
}

void window_resize_callback( int w, int h ) {
	/*glViewport(0,0,w,h);
	width = w;
	height = h;
	projection = glm::perspective(glm::radians(70.0f), (float)w/h, 0.3f, 100.0f);*/
}
