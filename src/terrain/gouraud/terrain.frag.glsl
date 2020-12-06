
#version 450 core

in vec3 light_intensity;

layout( location = 0 ) out vec4 frag_color;

in VERTEX_OUTPUT {
	vec4 position;
	vec2 heightmapUV;
	vec3 lightDir;	// .xyz
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
	float lightFactor;
	vec3 normal;
	vec4 world_pos;
	vec3 light_intensity;
	float morphLerpK;
} frag_in;

void main() {
	frag_color = vec4( frag_in.light_intensity, 1.0f );
}

