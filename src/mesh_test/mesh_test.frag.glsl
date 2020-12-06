
#version 450 core

const vec3 draw_color = vec3( 1.0f, 1.0f, 1.0f );

out vec4 fragment_color;

void main() {
	fragment_color = vec4( draw_color, 1.0f );
}
