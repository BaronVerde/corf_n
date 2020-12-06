
#version 450 core

// This is the position in the grid mesh, not world position !
layout( location = 0 ) in vec3 position;

// Texture with height values 0..1 ( * 65535 for real world values) above reference ellipsoid
layout( binding = 0 ) uniform sampler2D s_tile_heightmap;

uniform float u_height_factor = 1.0f;

// use linear filter manually. Not necessary if heightmap sampler is GL_LINEAR
// uniform bool u_useLinearFilter = false;
// --- Tile specific data (set on in the render loop on tile switch) ---
// Lower left world cartesian coordinate of heightmap tile
uniform vec3 u_tile_offset;
// Size x/y/z of heightmap tile in number of posts and max height.
uniform vec3 u_tile_scale;
// Max of terrain tile. Used to clamp triangles outside of horizontal texture range.
uniform vec2 u_tile_max;
// (width-1)/width, (height-1)/height. Width and height are the same.
uniform vec2 u_tile_to_texture;
// width, height, 1/width, 1/height in number of posts
uniform vec4 u_heightmap_texture_info;

// --- Node specific data. Set in the render loop for every npde ---
// @todo: these could be constants if all tiles are the same !
// .x = gridDim, .y = gridDimHalf, .z = oneOverGridDimHalf
uniform vec3 u_griddim;
// x and z hold horizontal minimums, .y holds the y center of the bounding box
uniform vec3 u_node_offset;
// x and z hold the horizontal scale of the bb in world size, .w holds the current lod level
uniform vec4 u_node_scale;
// distances for current lod level for begin and end of morphing
// @todo: These are static in the application for now. Make them dynamic.
uniform vec4 u_morph_consts;

uniform vec3 u_camera_position;
// Is equal to mvp matrix because terrain model m. is identity
uniform mat4 u_view_projection_matrix;

// Lighting
// equal to v-matrix because model matrix is identity
uniform mat4 u_model_view_matrix;
// uniform mat4 u_model_view_projection_matrix; equal to vp-matrix
uniform mat3 u_normal_matrix;

out VERTEX_OUTPUT {
	// unprojected position after morphing
	vec4 vertex_position;
	vec3 vertex_normal;
	vec4 view_space_position;
	vec3 view_space_normal;
	vec2 heightmap_uv;
	// .xyz = eyeDir, .w = eyeDist
	vec4 eyeDir;
	float morph_lerp_k;
} vert_out;

// Returns position relative to current tile fur texture lookup. Y value unsued.
vec3 get_tile_vertex_pos( vec3 position ) {
	vec3 ret_val = position * u_node_scale.xyz + u_node_offset;
	ret_val.xz = min( ret_val.xz, u_tile_max );
	return ret_val;
}

// Calculate texture coordinates for the heightmap. Observe lod node's offset and scale.
vec2 calculate_uv( vec2 vertex ) {
	vec2 heightmap_uv = ( vertex.xy - u_tile_offset.xz ) / u_tile_scale.xz;
	heightmap_uv *= u_tile_to_texture;
	heightmap_uv += u_heightmap_texture_info.zw * 0.5f;
	return heightmap_uv;
}

// morphs vertex .xy from high to low detailed mesh position
vec2 morph_vertex( vec3 pos, vec2 vertex, float morph_lerp_k ) {
	vec2 decimals = ( fract( pos.xz * vec2( u_griddim.y, u_griddim.y ) ) * 
					vec2( u_griddim.z, u_griddim.z ) ) * u_node_scale.xz;
	return vertex - decimals * morph_lerp_k;
}

// Assumes linear filtering being enabled in sampler
float sample_heightmap( vec2 uv ) {
	return texture( s_tile_heightmap, uv ).r;
}

// calculate vertex normal via central difference
vec3 calculate_normal( vec2 uv ) {
	vec2 texel_size = u_heightmap_texture_info.zw;
	float n = sample_heightmap( uv + vec2( 0.0f, -texel_size.x ) );
	float s = sample_heightmap( uv + vec2( 0.0f, texel_size.x ) );
	float e = sample_heightmap( uv + vec2( -texel_size.y, 0.0f ) );
	float w = sample_heightmap( uv + vec2( texel_size.y, 0.0f ) );
	vec3 sn = vec3( 0.0f , s - n, -( texel_size.y * 2.0f ) );
	vec3 ew = vec3( -( texel_size.x * 2.0f ), e - w, 0.0f );
	sn *= ( texel_size.y * 2.0f );
    ew *= ( texel_size.x * 2.0f );
    sn = normalize( sn );
    ew = normalize( ew );
    vec3 result = normalize( cross( sn, ew ) );
	return result;
}

void cdlod_vertex() {
	// calculate position on the heightmap for height value lookup
	vec3 vertex = get_tile_vertex_pos( position );
	// Pre-sample height to be able to precisely calculate morphing value.
	vec2 pre_uv = calculate_uv( vertex.xz );
	vertex.y = sample_heightmap( pre_uv ) * u_height_factor;
	float eyeDistance = distance( vertex, u_camera_position );
	vert_out.morph_lerp_k = 1.0f - clamp( u_morph_consts.z - eyeDistance * u_morph_consts.w, 0.0f, 1.0f );
	vertex.xz = morph_vertex( position, vertex.xz, vert_out.morph_lerp_k );
	vert_out.heightmap_uv = calculate_uv( vertex.xz );
	vertex.y = sample_heightmap( vert_out.heightmap_uv ) * u_height_factor;
	// calculate position in world coordinates with the formula:
	// world position = tileOffset + tileVertexPosition * cellsize
	vert_out.vertex_position = vec4( vertex, 1.0f );
	vert_out.view_space_position = u_model_view_matrix * vec4( vertex, 1.0f );
	vert_out.vertex_normal = calculate_normal( vert_out.heightmap_uv );
	vert_out.view_space_normal = normalize( u_normal_matrix * (vert_out.vertex_normal * u_tile_scale.xyz) );
	//vert_out.eyeDir = vec4( vert_out.view_space_position.xyz - u_camera_position, eyeDistance );
}

void main() {
	cdlod_vertex();
	// This would be the mvp matrix, but m-matrix is identity
    gl_Position = u_view_projection_matrix * vert_out.vertex_position;
}

