
#include "camera.h"
#include "window.h"
#include "base/logbook.h"
#include <stdio.h>
#include <tgmath.h>

// @todo: fix this struct, split it, setters and getters
static struct camera {
	vec3f position;
	// Orbiting camera target.
	vec3f target;
	float distance_to_target;
	float near_plane;
	float far_plane;
	mat4f view_matrix;
	mat4f perspective_matrix;
	// Perspective projection * view Matrix pre-comp.
	mat4f view_perspective_matrix;
	mat4f untranslated_view_perspective_matrix;
	//omath::mat4 m_zOffsetPerspectiveMatrix;
	vec3f front;
	vec3f up;
	// vec3d m_worldUp; // for physics
	vec3f right;
	// yaw, pitch and zoom in degrees
	float yaw;
	float pitch;
	float zoom;
	camera_mode_t mode;
	float movement_speed;
	//	Moving starts with key press, ends with key release.
	bool is_moving;
	// Movement direction if camera is moving in firstperson mode, relative to camera axes.
	direction_t direction;
	float mouse_sensitivity;
	view_frustum_t view_frustum;
} camera;

static void camera_calculate_fov();
static void camera_calculate_initial_angles();
static void camera_update_vectors();

inline void camera_create( const vec3f *const camera_position, const vec3f *const camera_target ) {
	camera.near_plane = 1.0f;
	camera.far_plane = 1000.0f;
	camera.zoom = 45.0f;
	// Initial up-vector in world space. Later it's calculated by crossing front and right
	vec3f world_up = { 0.0f,1.0f,0.0f };
	camera.up = world_up;
	camera.mode = FIRST_PERSON;
	camera.movement_speed = 50.0f;
	camera.is_moving = false;
	camera.mouse_sensitivity = 0.005f;
	// Set position and target and calculate vectors for camera space
	camera_set_position_and_target( camera_position, camera_target );
	camera_calculate_fov();
}

inline bool camera_mouse_move( double x, double y ) {
	bool handled = false;
	float xf = (float)x, yf = (float)y;
	camera.pitch += camera.mouse_sensitivity * ( window_get_center_y() - yf );
	if( FIRST_PERSON == camera.mode )
		camera.yaw -= camera.mouse_sensitivity * ( window_get_center_x() - xf );
	else
		camera.yaw += camera.mouse_sensitivity * ( window_get_center_x() - xf );
	// So screen doesn't get flipped
	if( camera.pitch > 89.0f )
		camera.pitch = 89.0f;
	if( camera.pitch < -89.0f )
		camera.pitch = -89.0f;
	handled = true;
	return handled;
}

inline bool camera_key_event( int key, int action, int mods ) {
	bool handled = false;
	// movement is mirrored between fps and orbiting mode
    if( GLFW_PRESS == action ) {
    	switch( key ) {
    		case GLFW_KEY_P:
    			camera_print_position();
    			handled = true;
    			break;
    		case GLFW_KEY_W:
    			camera.is_moving = true;
    			if( FIRST_PERSON == camera.mode ) {
    				if( mods & GLFW_MOD_SHIFT )
    					camera.direction = FAST_FORWARD;
    				else
    					camera.direction = FORWARD;
    			} else {
    				if( mods & GLFW_MOD_SHIFT )
    					camera.direction = FAST_CLOSE;
    				else
    					camera.direction = CLOSE;
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_S:
    			camera.is_moving = true;
    			if( FIRST_PERSON == camera.mode ) {
    				if( mods & GLFW_MOD_SHIFT )
    					camera.direction = FAST_BACKWARD;
    				else
    					camera.direction = BACKWARD;
    			} else {
    				if( mods & GLFW_MOD_SHIFT )
    					camera.direction = FAST_RETREAT;
    				else
    					camera.direction = RETREAT;
    			}
    			handled = true;
    			break;
    		case GLFW_KEY_A:
    			camera.is_moving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				camera.direction = FAST_LEFT;
    			else
    				camera.direction = LEFT;
    			handled = true;
    			break;
    		case GLFW_KEY_D:
    			camera.is_moving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				camera.direction = FAST_RIGHT;
    			else
    				camera.direction = RIGHT;
    			handled = true;
    			break;
    		case GLFW_KEY_Z:
    			camera.is_moving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				camera.direction = FAST_DOWN;
    			else
    				camera.direction = DOWN;
    			handled = true;
    			break;
    		case GLFW_KEY_X:
    			camera.is_moving = true;
    			if( mods & GLFW_MOD_SHIFT )
    				camera.direction = FAST_UP;
    			else
    				camera.direction = UP;
    			handled = true;
    			break;
    		case GLFW_KEY_Q:
    			camera.is_moving = true;
    			camera.direction = ROTATE_LEFT;
    			break;
    		case GLFW_KEY_E:
    			camera.is_moving = true;
    			camera.direction = ROTATE_RIGHT;
    			handled = true;
    			break;
    		case GLFW_KEY_M:
    			view_frustum_print(&camera.view_frustum);
    			handled = true;
    			break;
    		case GLFW_KEY_TAB:
    			// @todo
    			logbook_log( LOG_WARNING, "Camera mode first person only, for now" );
    			if( ORBITING == camera.mode ) {
    				// @todo Set initial values from orbiting cam
    				camera.mode = FIRST_PERSON;
    				camera_set_position_and_target( &camera.position, &camera.target );
    			} else {
    				//camera.mode = ORBITING;
    				//camera_update_vectors();
    			}
    			handled = true;
    			break;
    	}
    }
    if( GLFW_RELEASE == action && ( GLFW_KEY_W == key || GLFW_KEY_S == key ||
    								GLFW_KEY_A == key || GLFW_KEY_D == key ||
									GLFW_KEY_Z == key || GLFW_KEY_X == key ||
									GLFW_KEY_Q == key || GLFW_KEY_E == key ) ) {
    	camera.is_moving = false;
    	handled = true;
    }
    // If moving, camera vectors and frustum planes are updated in updateMoving()
    return handled;
}

inline void camera_update_moving( const float deltatime ) {
	if( camera.is_moving ) {
		float velocity = camera.movement_speed * deltatime;
		// up, front and right must be normalized or else you will be lost in space
		vec3f temp/* = {0.0f,0.0f,0.0f}*/;
		switch( camera.direction ) {
			case FAST_FORWARD:
				vec3f_mul_s( &camera.front, 10.0f * velocity, &temp );
				vec3f_add( &camera.position, &temp, &camera.position );
				//camera.position += camera.front * 10.0f * velocity;
				break;
			case FORWARD:
				vec3f_mul_s( &camera.front, velocity, &temp );
				vec3f_add( &camera.position, &temp, &camera.position );
				//camera.position += camera.front * velocity;
				break;
			case FAST_BACKWARD:
				vec3f_mul_s( &camera.front, 10.0f * velocity, &temp );
				vec3f_sub( &camera.position, &temp, &camera.position );
				//camera.position -= camera.front * 10.0f * velocity;
				break;
			case BACKWARD:
				vec3f_mul_s( &camera.front, velocity, &temp );
				vec3f_sub( &camera.position, &temp, &camera.position );
				//camera.position -= camera.front * velocity;
				break;
			case FAST_LEFT:
				vec3f_mul_s( &camera.right, 10.0f * velocity, &temp );
				vec3f_sub( &camera.position, &temp, &camera.position );
				//camera.position -= camera.right * 10.0f * velocity;
				break;
			case LEFT:
				vec3f_mul_s( &camera.right, velocity, &temp );
				vec3f_sub( &camera.position, &temp, &camera.position );
				//camera.position -= camera.right * velocity;
				break;
			case FAST_RIGHT:
				vec3f_mul_s( &camera.right, 10.0f * velocity, &temp );
				vec3f_add( &camera.position, &temp, &camera.position );
				//camera.position += camera.right * 10.0f * velocity;
				break;
			case RIGHT:
				vec3f_mul_s( &camera.right, velocity, &temp );
				vec3f_add( &camera.position, &temp, &camera.position );
				//camera.position += camera.right * velocity;
				break;
			case FAST_UP:
				vec3f_mul_s( &camera.up, 10.0f * velocity, &temp );
				vec3f_add( &camera.position, &temp, &camera.position );
				//camera.position += camera.up * 10.0f * velocity;
				break;
			case UP:
				vec3f_mul_s( &camera.up, velocity, &temp );
				vec3f_add( &camera.position, &temp, &camera.position );
				//camera.position += camera.up * velocity;
				break;
			case FAST_DOWN:
				vec3f_mul_s( &camera.up, 10.0f * velocity, &temp );
				vec3f_sub( &camera.position, &temp, &camera.position );
				//camera.position -= camera.up * 10.0f * velocity;
				break;
			case DOWN:
				vec3f_mul_s( &camera.up, velocity, &temp );
				vec3f_sub( &camera.position, &temp, &camera.position );
				//camera.position -= camera.up * velocity;
				break;
			case ROTATE_LEFT:
				break;
			case ROTATE_RIGHT:
				break;
			case FAST_CLOSE:
				camera.distance_to_target -= 10.0f * velocity;
				break;
			case CLOSE:
				camera.distance_to_target -= velocity;
				break;
			case FAST_RETREAT:
				camera.distance_to_target += 10.0f * velocity;
				break;
			case RETREAT:
				camera.distance_to_target += velocity;
				break;
		}
	}
	// update camera matrices and vectors and frustum vectors
	camera_update_vectors();
}

inline void camera_set_position_and_target( const vec3f* const pos, const vec3f* const target ) {
	vec3f_set_from( pos, &camera.position );
	vec3f_set_from( target, &camera.target );
	camera_calculate_initial_angles();
}

inline void camera_set_near_far_plane( const float near, const float far ) {
	camera.near_plane = near;
	camera.far_plane = far;
	// update view matrix and frustum
	camera_calculate_fov();
}

inline void camera_set_movement_speed( const float speed ) {
	camera.movement_speed = speed;
}

inline void camera_print_position() {
	char msg[LOGBOOK_MAX_MSG_LEN];
	sprintf( msg, "Cam position: %.2f/%.2f/%.2f; target: %.2f/%.2f/%.2f; mode: %s",
			camera.position.x, camera.position.y, camera.position.z,
			camera.target.x, camera.target.y, camera.target.z,
			camera.mode == FIRST_PERSON ? "first person" : "orbiting" );
	logbook_log( LOG_INFO, msg );
	sprintf( msg, "Cam front: %.2f/%.2f/%.2f; up: %.2f/%.2f/%.2f; right: %.2f/%.2f/%.2f",
			camera.front.x, camera.front.y, camera.front.z,
			camera.up.x, camera.up.y, camera.up.z,
			camera.right.x, camera.right.y, camera.right.z );
	logbook_log( LOG_INFO, msg );
}

inline float camera_get_near_plane() {
	return camera.near_plane;
}

inline float camera_get_far_plane() {
	return camera.far_plane;
}

inline view_frustum_t *camera_get_view_frustum() {
	return &camera.view_frustum;
}

inline vec3f *camera_get_position() {
	return &camera.position;
}

inline mat4f *camera_get_view_projection_matrix() {
	return &camera.view_perspective_matrix;
}

// -------------- statics ------------------
inline void camera_calculate_fov() {
	mat4f_perspective(
			radiansf(camera.zoom),
			window_get_width() / window_get_height(),
			camera.near_plane,
			camera.far_plane,
			&camera.perspective_matrix
	);
	/*const float zModMul{ 1.001f };
	const float zModAdd{ 0.01f };
	m_zOffsetPerspectiveMatrix = omath::perspective( omath::radians( m_zoom ),
			(float)m_window->getWidth() / (float)m_window->getHeight(),
			m_nearPlane * zModMul + zModAdd, m_farPlane * zModMul + zModAdd );*/
	frustum_set_fov(
			camera.zoom,
			window_get_width() / window_get_height(),
			camera.near_plane,
			camera.far_plane,
			&camera.view_frustum
	);
}

inline void camera_calculate_initial_angles() {
	// Calculate initial yaw and pitch on change of position or target
	// @todo different cases for camera mode
	vec3f direction;
	vec3f_sub( &camera.target, &camera.position, &direction );
	camera.distance_to_target = vec3f_magnitude( &direction );
	camera.yaw = degreesf( atan2( -direction.x, direction.z ) );
	camera.pitch = degreesf(
			atan2( direction.y, sqrt( ( direction.x * direction.x) + ( direction.y * direction.y ) ) )
	);
	camera_update_vectors();
}

inline void camera_update_vectors() {
	if( ORBITING == camera.mode ) {
		//m_distanceToTarget = omath::length( m_target - m_position );
		/*const float yaw = -radiansf(camera.yaw);
		const float pitch = -radiansf(camera.pitch);
		const float cp = cos(pitch);
		// Calculate the camera position on the sphere around the target
		vec3f pos = {
				camera.distance_to_target * sin(yaw) * cp,
				camera.distance_to_target * sin(pitch),
				-camera.distance_to_target * cos(yaw) * cp,
		};
		camera.position = pos;
		vec3f temp;
		vec3f_normalize( vec3f_sub( &camera.target, &camera.position, &temp ), &camera.front );
		mat4f_lookat( &camera.position, &camera.target, &camera.up, &camera.view_matrix );
		frustum_set_camera_vectors(
				&camera.position, &camera.target, &camera.up, &camera.view_frustum
		);*/
	} else {
		// flip yaw and pitch because intuition (at least mine ;-))
		const float yaw = radiansf(camera.pitch);
		const float pitch = radiansf(camera.yaw);
		const float cy = cos(yaw);
		// Calculate the new front and right vectors. Camera position is set by movement.
		const vec3f temp_front = { -cy * sin(pitch), sin(yaw), cy * cos(pitch) };
		vec3f_normalize( &temp_front, &camera.front );
		vec3f front;
		vec3f_add( &camera.position, &temp_front, &front );
		// set view matrix and update frustum vectors
		mat4f_lookat( &camera.position, &front, &camera.up, &camera.view_matrix );
		frustum_set_camera_vectors(
				&camera.position, &front, &camera.up, &camera.view_frustum
		);
	}
	// Also re-calculate the right vector ...
	vec3f temp;
	vec3f_normalize( vec3f_cross( &camera.front, &camera.up, &temp ), &camera.right );
	// ... and the up-vector in case pitch has changed
	//vec3f_normalize( vec3f_cross( &camera.right, &camera.front, &temp ), &camera.up );
	// prefab projection * view
	mat4f_mul( &camera.perspective_matrix, &camera.view_matrix, &camera.view_perspective_matrix );
	// prefab proj * untranslated view matrix, stripped of translation
	/*mat4 temp_mat;
	mat4_from( &temp_mat, &camera.view_matrix );
	// strip last column (translation) from matrix
	temp_mat.data[12] = 0.0f; temp_mat.data[13] = 0.0f; temp_mat.data[14] = 0.0f; temp_mat.data[15] = 1.0f;
	mat4_mul( &camera.untranslated_view_perspective_matrix, &camera.perspective_matrix, &temp_mat );*/
}
