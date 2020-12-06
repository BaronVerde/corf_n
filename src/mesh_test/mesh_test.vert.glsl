
#version 450 core

layout( location = 0 ) in vec3 position;

layout( location = 5 ) uniform vec3 u_camera_position;
layout( location = 7 ) uniform mat4 u_model_matrix;
layout( location = 15 ) uniform mat4 u_view_projection_matrix;

void main() {
	gl_Position = u_view_projection_matrix * u_model_matrix * vec4( position, 1.0 );
}
