
#include <stdio.h>
#include <string.h>
#include <tgmath.h>
#include "logbook.h"
#include "window.h"

// local prototypes
static void window_error_callback( int error, const char *msg );

static void APIENTRY glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *message, const void *userParam );

static void window_key_callback( GLFWwindow *window, int key, int scancode, int action, int mods );

static void window_cursor_pos_callback( GLFWwindow *window, double xPos, double yPos );

// only on near/for/zoom change
static void window_calculate_fov();

// on position/target set, calculate yaw and pitch
static void window_calculate_initial_angles();

static void window_update_vectors();

// Terrain will be a first person camera with double precision view matrix
typedef enum {
	ORBITING = 0, FIRST_PERSON, TERRAIN
} camera_mode_t;

typedef enum {
	// Movement of first person camera
	FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN, ROTATE_LEFT, ROTATE_RIGHT,
	// Faster ...
	FAST_FORWARD, FAST_BACKWARD, FAST_RIGHT, FAST_LEFT, FAST_UP, FAST_DOWN,
	// Movement of orbiting camera
	CLOSE, RETREAT, FAST_CLOSE, FAST_RETREAT
} movement_direction_t;

static struct window_t {
	int width;
	int height;
	bool vsync;
	bool cursor_disabled;
	bool debug_enabled;
	char title[WINDOW_MAX_TITLE_LENGTH];
	GLFWwindow *window;
	camera_mode_t camera_mode;
	vec3f camera_position;
	// Orbiting camera target.
	vec3f camera_target;
	float distance_to_target;
	float near_plane;
	float far_plane;
	float zoom;
	mat4f view_matrix;
	mat4f perspective_matrix;
	// Perspective projection * view Matrix pre-comp.
	mat4f view_perspective_matrix;
	//mat4f untranslated_view_perspective_matrix;
	//mat4f z_axis_offset_perspective_matrix;
	vec3f camera_front;
	vec3f camera_up;
	vec3f camera_right;
	// vec3d m_worldUp; // for physics
	// Angles in degrees
	float camera_yaw;
	float camera_pitch;
	float movement_speed;
	//	Moving starts with key press, ends with key release.
	bool camera_is_moving;
	// Movement direction if camera is moving in firstperson mode, relative to camera axes.
	movement_direction_t movement_direction;
	float mouse_sensitivity;
	view_frustum_t view_frustum;
} gw_window;

// Interface functions @todo error checking and return values
void window_create(
		const char* title, const int width, const int height,
		const vec3f *const pos, const vec3f *const target ) {
	gw_window.window = NULL;
	gw_window.debug_enabled = true;
	gw_window.width = width;
	gw_window.height = height;
	strncpy( gw_window.title, title, WINDOW_MAX_TITLE_LENGTH-1 );
	gw_window.cursor_disabled = false;
	gw_window.vsync = true;
	glfwSetErrorCallback( window_error_callback );
	if( GL_TRUE != glfwInit() ) {
		logbook_log( LOG_ERROR, "Error initialising glfw" );
		return;
	}
	// Window and context; we want OpenGL 4.5
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
	if( gw_window.debug_enabled )
		glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
	glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_SAMPLES, 0 );
	glfwWindowHint( GLFW_DEPTH_BITS, 32 );
	gw_window.window = glfwCreateWindow( gw_window.width, gw_window.height, gw_window.title, NULL, NULL );
	if( NULL == gw_window.window ) {
		logbook_log( LOG_ERROR, "Could not create glfw window." );
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent( gw_window.window );
	// Load gl function pointers
	if( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) ) {
		logbook_log( LOG_ERROR, "Error initialising glad" );
		glfwDestroyWindow( gw_window.window );
		glfwTerminate();
		return;
	}
	glfwSwapInterval( gw_window.vsync ? 1 : 0 );
	if( gw_window.debug_enabled ) {
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
			logbook_log( LOG_WARNING, "OpenGL 4.5 debug context not created. Continuing without debug messages." );
	}
	// Ensure we can capture key events
	glfwSetInputMode( gw_window.window, GLFW_STICKY_KEYS, GLFW_TRUE );
	// initially enable cursor, left alt-key switches
	glfwSetInputMode( gw_window.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
	glfwSetCursorPosCallback( gw_window.window, window_cursor_pos_callback );
	glfwSetKeyCallback( gw_window.window, window_key_callback );
	// init camera part
	gw_window.near_plane = 1.0f;
	gw_window.far_plane = 1000.0f;
	vec3f_set( 0.0f, 1.0f, 0.0f, &gw_window.camera_up );
	// zoom, yaw and pitch for internal calc in radians
	gw_window.camera_yaw = 0.0f;
	gw_window.camera_pitch = 0.0f;
	gw_window.zoom = radiansf( 45.0f );
	gw_window.camera_mode = ORBITING;
	gw_window.movement_speed = 30.0f;
	gw_window.camera_is_moving = false;
	gw_window.mouse_sensitivity = 0.0005f;
	window_set_position_and_target( pos, target );
	window_calculate_fov();
	window_update_vectors();
}

void window_delete() {
	glfwDestroyWindow( gw_window.window );
	gw_window.window = NULL;
	glfwTerminate();
}

// call once per frame
void window_update_moving( float deltatime ) {
	if( !gw_window.cursor_disabled )
		return;
	if( gw_window.camera_is_moving ) {
		float velocity = gw_window.movement_speed * deltatime;
		switch( gw_window.movement_direction ) {
		// up, front and right must be unit vectors or else you will be lost in space.
		vec3f temp;
		case FAST_FORWARD:
			vec3f_mul_s( &gw_window.camera_front, 10.0f * velocity, &temp );
			vec3f_add( &gw_window.camera_position, &temp, &gw_window.camera_position );
			//glfw_window.camera_position += glfw_window.camera_front * 10.0f * velocity;
			break;
		case FORWARD:
			vec3f_mul_s( &gw_window.camera_front, velocity, &temp );
			vec3f_add( &gw_window.camera_position, &temp, &gw_window.camera_position );
			//glfw_window.camera_position += glfw_window.camera_front * velocity;
			break;
		case FAST_BACKWARD:
			vec3f_mul_s( &gw_window.camera_front, 10.0f * velocity, &temp );
			vec3f_sub( &gw_window.camera_position, &temp, &gw_window.camera_position );
			//glfw_window.camera_position -= glfw_window.camera_front * 10.0f * velocity;
			break;
		case BACKWARD:
			vec3f_mul_s( &gw_window.camera_front, velocity, &temp );
			vec3f_sub( &gw_window.camera_position, &temp, &gw_window.camera_position );
			//glfw_window.camera_position -= glfw_window.camera_front * velocity;
			break;
		case FAST_LEFT:
			//glfw_window.camera_position -= glfw_window.right * 10.0f * velocity;
			break;
		case LEFT:
			//glfw_window.camera_position -= glfw_window.right * velocity;
			break;
		case FAST_RIGHT:
			//glfw_window.camera_position += glfw_window.right * 10.0f * velocity;
			break;
		case RIGHT:
			//glfw_window.camera_position += glfw_window.right * velocity;
			break;
		case FAST_UP:
			//glfw_window.camera_position += glfw_window.up * 10.0f * velocity;
			break;
		case UP:
			//glfw_window.camera_position += glfw_window.up * velocity;
			break;
		case FAST_DOWN:
			//glfw_window.camera_position -= glfw_window.up * 10.0f * velocity;
			break;
		case DOWN:
			//glfw_window.camera_position -= glfw_window.up * velocity;
			break;
		case ROTATE_LEFT:
			break;
		case ROTATE_RIGHT:
			break;
		case FAST_CLOSE:
			gw_window.distance_to_target -= 10.0f * velocity;
			break;
		case CLOSE:
			gw_window.distance_to_target -= velocity;
			break;
		case FAST_RETREAT:
			gw_window.distance_to_target += 10.0f * velocity;
			break;
		case RETREAT:
			gw_window.distance_to_target += velocity;
			break;
		}
	}
	window_update_vectors();
}

void window_set_position_and_target( const vec3f* const pos, const vec3f* const target)  {
	vec3f_set_from( pos, &gw_window.camera_position );
	vec3f_set_from( target, &gw_window.camera_target );
	window_calculate_initial_angles();
}

void window_print_camera_position() {
	printf( "Cam position: %.2f/%.2f/%.2f; target: %.2f/%.2f/%.2f; front: %.2f/%.2f/%.2f; mode: %s\n",
			gw_window.camera_position.x, gw_window.camera_position.y, gw_window.camera_position.z,
			gw_window.camera_target.x, gw_window.camera_target.y, gw_window.camera_target.z,
			gw_window.camera_front.x, gw_window.camera_front.y, gw_window.camera_front.z,
			gw_window.camera_mode == FIRST_PERSON ? "first person" : "orbiting" );
}

inline GLFWwindow *window_get_window() {
	return gw_window.window;
}

inline vec3f *window_get_camera_position() {
	return &gw_window.camera_position;
}

inline mat4f *window_get_view_matrix() {
	return &gw_window.view_matrix;
}

inline mat4f *window_get_view_perspective_matrix() {
	return &gw_window.view_perspective_matrix;
}


// ------------------ statics below ----------------------
void window_error_callback( int error, const char *msg ) {
	char m[LOGBOOK_MAX_MSG_LEN];
	snprintf( m, LOGBOOK_MAX_MSG_LEN-1, "[%d] %s", error, msg );
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
	char m[LOGBOOK_MAX_MSG_LEN] = "Source: ";
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
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strcat( m, "deprecated behaviour" ); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: strcat( m, "undefined behaviour" ); break;
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
    if( strlen( m ) + strlen( message ) >= LOGBOOK_MAX_MSG_LEN )
    	logbook_log( LOG_UNSPECIFIED, "Gl debug output: message too long. Message truncated. Pls. split." );
    strncat( m, message, LOGBOOK_MAX_MSG_LEN-1 );
    logbook_log( LOG_UNSPECIFIED, m );
    if( NULL != userParam )
    	logbook_log( LOG_INFO, "Additional userdata not displayed" );
}

void window_key_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	if( GLFW_PRESS == action || GLFW_REPEAT == action ) {
		switch( key ) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose( gw_window.window, GLFW_TRUE );
			break;
		case GLFW_KEY_LEFT_ALT:
			gw_window.cursor_disabled = !gw_window.cursor_disabled;
			glfwSetInputMode(
					gw_window.window, GLFW_CURSOR,
					gw_window.cursor_disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
			);
			break;
		case GLFW_KEY_V:
			gw_window.vsync = !gw_window.vsync;
			glfwSwapInterval( gw_window.vsync ? 1 : 0 );
			break;
			// movement is mirrored between fps and orbiting mode
		case GLFW_KEY_P:
			window_print_camera_position();
			break;
		case GLFW_KEY_W:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				gw_window.camera_is_moving = true;
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_FORWARD;
				else
					gw_window.movement_direction = FORWARD;
			} else {
				gw_window.camera_is_moving = true;
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_CLOSE;
				else
					gw_window.movement_direction = CLOSE;
			}
			break;
		case GLFW_KEY_S:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				gw_window.camera_is_moving = true;
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_BACKWARD;
				else
					gw_window.movement_direction = BACKWARD;
			} else {
				gw_window.camera_is_moving = true;
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_RETREAT;
				else
					gw_window.movement_direction = RETREAT;
			}
			break;
		case GLFW_KEY_A:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_LEFT;
				else
					gw_window.movement_direction = LEFT;
				gw_window.camera_is_moving = true;
			}
			break;
		case GLFW_KEY_D:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				gw_window.camera_is_moving = true;
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_RIGHT;
				else
					gw_window.movement_direction = RIGHT;
			}
			break;
		case GLFW_KEY_Z:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				gw_window.camera_is_moving = true;
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_DOWN;
				else
					gw_window.movement_direction = DOWN;
			}
			break;
		case GLFW_KEY_X:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				gw_window.camera_is_moving = true;
				if( mods & GLFW_MOD_SHIFT )
					gw_window.movement_direction = FAST_UP;
				else
					gw_window.movement_direction = UP;
			}
			break;
		case GLFW_KEY_Q:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				gw_window.camera_is_moving = true;
				gw_window.movement_direction = ROTATE_LEFT;
			}
			break;
		case GLFW_KEY_E:
			if( FIRST_PERSON == gw_window.camera_mode ) {
				gw_window.camera_is_moving = true;
				gw_window.movement_direction = ROTATE_RIGHT;
			}
			break;
		case GLFW_KEY_TAB:
			if( ORBITING == gw_window.camera_mode ) {
				// @todo Set initial values from orbiting cam
				gw_window.camera_mode = FIRST_PERSON;
				window_set_position_and_target(
						&gw_window.camera_position, &gw_window.camera_target
				);
			} else
				gw_window.camera_mode = ORBITING;
			window_update_vectors();
			break;
		}
	}
	if( GLFW_RELEASE == action && ( GLFW_KEY_W == key || GLFW_KEY_S == key ||
									GLFW_KEY_A == key || GLFW_KEY_D == key ||
									GLFW_KEY_Z == key || GLFW_KEY_X == key ||
									GLFW_KEY_Q == key || GLFW_KEY_E == key ) )
		gw_window.camera_is_moving = false;
	// If moving, camera vectors and frustum planes are updated in update_moving()
}

void window_cursor_pos_callback( GLFWwindow *window, double x, double y ) {
	if( !gw_window.cursor_disabled )
		return;
	const float threshold = radiansf( 89.0f );
	float xf = (float)x, yf = (float)y;
	// Don't know why this is necessary ...
	glfwSetCursorPos( gw_window.window, (double)gw_window.width/2.0, (double)gw_window.height/2.0 );
	gw_window.camera_pitch += gw_window.mouse_sensitivity * ( (float)gw_window.height/2.0f - yf );
	if( FIRST_PERSON == gw_window.camera_mode )
		gw_window.camera_yaw -= gw_window.mouse_sensitivity * ( (float)gw_window.width/2.0f - xf );
	else
		gw_window.camera_yaw += gw_window.mouse_sensitivity * ( (float)gw_window.width/2.0f - xf );
	// So screen doesn't get flipped
	if( gw_window.camera_pitch > threshold )
		gw_window.camera_pitch = threshold;
	if( gw_window.camera_pitch < -threshold )
		gw_window.camera_pitch = -threshold;
}
#pragma GCC diagnostic pop

void window_calculate_initial_angles() {
	// Calculate initial yaw and pitch on change of position or target
	// @todo different cases for camera mode
	vec3f direction;
	vec3f_sub( &gw_window.camera_target, &gw_window.camera_position, &direction );
	gw_window.distance_to_target = vec3f_magnitude( &direction );
	gw_window.camera_yaw = atan2( -direction.x, direction.z );
	gw_window.camera_pitch = atan2(
			direction.y, sqrt( ( direction.x * direction.x) + ( direction.y * direction.y ) )
	);
	window_update_vectors();
}

void window_calculate_fov() {
	mat4f_perspective(
			gw_window.zoom,
			(float)gw_window.width / (float)gw_window.height,
			gw_window.near_plane,
			gw_window.far_plane,
			&gw_window.perspective_matrix
	);
	/*const float zModMul{ 1.001f };
	const float zModAdd{ 0.01f };
	m_zOffsetPerspectiveMatrix = omath::perspective( m_zoom,
			(float)m_window->getWidth() / (float)m_window->getHeight(),
			m_nearPlane * zModMul + zModAdd, m_farPlane * zModMul + zModAdd );*/
	frustum_set_fov(
			gw_window.zoom,
			(float)gw_window.width / (float)gw_window.height,
			gw_window.near_plane,
			gw_window.far_plane,
			&gw_window.view_frustum
	);
}

void window_update_vectors() {
	if( ORBITING == gw_window.camera_mode ) {
		//m_distanceToTarget = omath::length( m_target - m_position );
		const float cp = cos( -gw_window.camera_pitch );
		// Calculate the camera position on the sphere around the target
		vec3f_set(
				gw_window.distance_to_target * sin( -gw_window.camera_yaw ) * cp,
				gw_window.distance_to_target * sin( -gw_window.camera_pitch ),
				-gw_window.distance_to_target * cos( -gw_window.camera_yaw ) * cp,
				&gw_window.camera_position
		);
		vec3f temp;
		vec3f_normalize( vec3f_sub(
				&gw_window.camera_target, &gw_window.camera_position, &temp ), &gw_window.camera_front
		);
		// conversion to float because precision isn't needed here
		mat4f_lookat(
				&gw_window.camera_position, &gw_window.camera_target, &gw_window.camera_up,
				&gw_window.view_matrix
		);
		frustum_set_camera_vectors(
				&gw_window.camera_position, &gw_window.camera_target, &gw_window.camera_up,
				&gw_window.view_frustum
		);
	} else {
		// yaw and pitch are flipped in fp mode because intuition (at least mine ;-))
		const float cy = cos( gw_window.camera_pitch );
		// Calculate the new front and right vectors. Camera position is set by movement.
		vec3f temp_front;
		vec3f_set(
				-cy * sin( gw_window.camera_yaw ),
				sin( gw_window.camera_pitch ),
				cy * cos( gw_window.camera_yaw ),
				&temp_front
		);
		vec3f_normalize( &temp_front, &gw_window.camera_front );
		vec3f looking_direction;
		// @todo normalize ?
		vec3f_add( &gw_window.camera_position, &temp_front, &looking_direction );
		mat4f_lookat(
				&gw_window.camera_position, &looking_direction, &gw_window.camera_up,
				&gw_window.view_matrix
		);
		frustum_set_camera_vectors(
				&gw_window.camera_position, &looking_direction, &gw_window.camera_up,
				&gw_window.view_frustum
		);
	}
	// Also re-calculate the right vector
	vec3f temp;
	vec3f_normalize( vec3f_cross(
			&gw_window.camera_front, &gw_window.camera_up, &temp ), &gw_window.camera_right
	);
	// prefab projection * view
	mat4f_mul(
			&gw_window.perspective_matrix, &gw_window.view_matrix, &gw_window.view_perspective_matrix
	);
	// prefab proj * untranslated view matrix, stripped of translation
	/*mat4f temp_mat;
	mat4f_from( &gw_window.view_matrix, &temp_mat );
	// strip last column (translation) from matrix
	temp_mat.data[12] = 0.0f; temp_mat.data[13] = 0.0f; temp_mat.data[14] = 0.0f; temp_mat.data[15] = 1.0f;
	mat4f_mul( &gw_window.perspective_matrix, &temp_mat, &gw_window.untranslated_view_perspective_matrix );*/
}
