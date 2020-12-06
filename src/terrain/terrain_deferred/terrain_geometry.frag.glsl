
#version 450 core

// G-buffer textures
layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_spec;

in VERTEX_OUTPUT {
	vec4 frag_pos;
	vec4 position;
	vec3 normal;
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
} fragIn;

//uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;
uniform vec4 diffuse_color = vec4( 0.4f, 0.4f, 0.8f, 1.0f );

void main() {    
	// store the fragment position vector in the first g-buffer texture
	//g_position = vec3(fragIn.position.xyz);
	g_position = vec3(fragIn.position.xyz);
	// also store the per-fragment normals into the g-buffer (already normalized !)
	g_normal = normalize(fragIn.normal);
	// and the diffuse per-fragment color
	// g_albedo_spec.rgb = texture(texture_diffuse1, TexCoords).rgb;
	g_albedo_spec.rgb = vec3(diffuse_color.rgb);
	// store specular intensity in gAlbedoSpec's alpha component
	// g_albedo_spec.a = texture(texture_specular1, TexCoords).r;
	g_albedo_spec.a = 16.0f;
}

