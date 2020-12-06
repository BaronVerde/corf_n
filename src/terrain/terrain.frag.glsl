
// Source: David Wolf, GLSL Cookbook, 3rd ed., pp 130ff

#version 450 core

const float PI = 3.1415926536f;
const vec3 GAMMA = vec3(1.0f/2.2f);

// Light position in cam. coords.
uniform vec4 u_light_position;
uniform vec3 u_light_intensity;

// The terrain material
uniform float u_terrain_roughness;
// Metallic (true) or dielectric (false). Terrain is not
uniform bool u_terrain_metal;
// Diffuse color for dielectrics, f0 for metallic
uniform vec3 u_terrain_color;

in VERTEX_OUTPUT {
	// unprojected position after morphing
	vec4 vertex_position;
	vec3 vertex_normal;
	vec4 view_space_position;
	vec3 view_space_normal;
	vec2 heightmap_uv;
	// .xyz = eyeDir, .w = eyeDist
	vec4 eyeDir;
	float morph_lerp_k;
} frag_in;

layout (location=0) out vec4 frag_color;

// BRDF Bidirectional reflectance distribution function

// GGX Trowbridge-Reitz
float ggx_distribution( const float normal_dot_halfway ) {
	const float alpha2 = u_terrain_roughness * u_terrain_roughness * u_terrain_roughness * u_terrain_roughness;
	const float d = (normal_dot_halfway*normal_dot_halfway) * (alpha2-1.0f) + 1.0f;
	return alpha2 / (PI*d*d);
}

float geometry_smith( const float dot_prod ) {
	const float k = (u_terrain_roughness+1.0f)*(u_terrain_roughness+1.0f) / 8.0f;
	const float denom = dot_prod*(1.0f-k) + k;
	return 1.0f / denom;
}

vec3 schlick_fresnel( const float lightdir_dot_halfway ) {
	vec3 specular_reflectance = vec3(0.04f);
	if( u_terrain_metal )
		specular_reflectance = u_terrain_color;
	return specular_reflectance + (1.0f-specular_reflectance) * pow(1.0f-lightdir_dot_halfway, 5);
}

vec3 microfacet_model( const vec3 position, const vec3 normal ) {
	// Metallic:
	vec3 diffuse_brdf;
	if( u_terrain_metal )
		diffuse_brdf = vec3(0.0f);
	else
		diffuse_brdf = u_terrain_color;
	vec3 light_direction = vec3(0.0f);
	vec3 light_i = u_light_intensity;
	// Directional light
	if( abs(u_light_position.w) < 0.01f )
		light_direction = normalize(u_light_position.xyz);
	// Positional light
	else {
		light_direction = u_light_position.xyz - position;
		const float dist = length(light_direction);
		light_direction = normalize(light_direction);
		light_i /= dist*dist;
	}
	const vec3 v = normalize(-position);
	const vec3 h = normalize(v+light_direction);
	const float normal_dot_h = dot(normal,h);
	const float lightdir_dot_h = dot(light_direction,h);
	const float normal_dot_lightdir = max( dot(normal,light_direction), 0.0f );
	const float normal_dot_v = dot(normal,v);
	const vec3 specular_brdf = 0.25f * ggx_distribution(normal_dot_h) * 
		schlick_fresnel(lightdir_dot_h) * geometry_smith(normal_dot_lightdir) * geometry_smith(normal_dot_v);
	return (diffuse_brdf + PI * specular_brdf) * light_i * normal_dot_lightdir;
}

void main() {
	// For each light source and add up
	vec3 col = microfacet_model(
		frag_in.view_space_position.xyz, normalize(frag_in.vertex_normal)
	);
	// Gamma 
	col = pow( col, GAMMA );
	frag_color = vec4( col, 1.0f );
}

