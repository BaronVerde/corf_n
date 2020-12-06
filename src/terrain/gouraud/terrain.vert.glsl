
#version 450 core

// This is the position in the grid mesh, not world position !
layout( location = 0 ) in vec3 position;

// Texture with height values 0..1 ( * 65535 for real world values) above reference ellipsoid
layout( binding = 0 ) uniform sampler2D g_tileHeightmap;

uniform float u_height_factor = 1.0f;

// use linear filter manually. Not necessary if heightmap sampler is GL_LINEAR
// uniform bool u_useLinearFilter = false;
// --- Tile specific data (set on in the render loop on tile switch) ---
// Lower left world cartesian coordinate of heightmap tile
uniform vec3 g_tileOffset;
// Size x/y/z of heightmap tile in number of posts and max height.
uniform vec3 g_tileScale;
// Max of terrain tile. Used to clamp triangles outside of horizontal texture range.
uniform vec2 g_tileMax;
// (width-1)/width, (height-1)/height. Width and height are the same.
uniform vec2 g_tileToTexture;
// width, height, 1/width, 1/height in number of posts
uniform vec4 g_heightmapTextureInfo;

// --- Node specific data. Set in the render loop for every npde ---
// @todo: these could be constants if all tiles are the same !
// .x = gridDim, .y = gridDimHalf, .z = oneOverGridDimHalf
uniform vec3 g_gridDim;
// x and z hold horizontal minimums, .y holds the y center of the bounding box
uniform vec3 g_nodeOffset;
// x and z hold the horizontal scale of the bb in world size, .w holds the current lod level
uniform vec4 g_nodeScale;
// distances for current lod level for begin and end of morphing
// @todo: These are static in the application for now. Make them dynamic.
uniform vec4 g_morphConsts;
layout (location = 5) uniform vec3 u_camera_position;
// Is equal to mvp matrix because terrain model m. is identity
layout (location = 15) uniform mat4 u_view_projection_matrix;

// Lighting
layout (location = 9) uniform mat4 u_model_view_matrix;
layout (location = 10) uniform mat4 u_model_view_projection_matrix;
layout (location = 19) uniform mat3 u_normal_matrix;
uniform struct u_light_info {
	// Light position in eye coords
	vec4 position;
	// Ambient light intensity
	vec3 la;
	// Diffuse light intensity
	vec3 ld;
	// Specular light intensity
	vec3 ls;
} light;
uniform struct u_material_info {
	// Ambient reflectivity
	vec3 ka;
	// Diffuse reflectivity
	vec3 kd;
	// Specular reflectivity
	vec3 ks;
	// Specular shininess factor
	float shininess;
} material;

out VERTEX_OUTPUT {
	vec4 position;
	vec2 heightmapUV;
	vec3 lightDir;	// .xyz
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
	float lightFactor;
	vec3 normal;
	vec4 world_pos;
	vec3 light_intensity;
	float morphLerpK;
} vert_out;

// Returns position relative to current tile fur texture lookup. Y value unsued.
vec3 getTileVertexPos( vec3 inPosition ) {
	vec3 returnValue = inPosition * g_nodeScale.xyz + g_nodeOffset;
	returnValue.xz = min( returnValue.xz, g_tileMax );
	return returnValue;
}

// Calculate texture coordinates for the heightmap. Observe lod node's offset and scale.
vec2 calculateUV( vec2 vertex ) {
	vec2 heightmapUV = ( vertex.xy - g_tileOffset.xz ) / g_tileScale.xz;
	heightmapUV *= g_tileToTexture;
	heightmapUV += g_heightmapTextureInfo.zw * 0.5f;
	return heightmapUV;
}

// morphs vertex .xy from high to low detailed mesh position
vec2 morphVertex( vec3 inPosition, vec2 vertex, float morphLerpValue ) {
	vec2 decimals = ( fract( inPosition.xz * vec2( g_gridDim.y, g_gridDim.y ) ) * 
					vec2( g_gridDim.z, g_gridDim.z ) ) * g_nodeScale.xz;
	return vertex - decimals * morphLerpValue;
}

// Assumes linear filtering being enabled in sampler
float sampleHeightmap( vec2 uv ) {
	return texture( g_tileHeightmap, uv ).r;
}

// calculate vertex normal via central difference
vec3 calculateNormal( vec2 uv ) {
	vec2 texelSize = g_heightmapTextureInfo.zw;
	float n = sampleHeightmap( uv + vec2( 0.0f, -texelSize.x ) );
	float s = sampleHeightmap( uv + vec2( 0.0f, texelSize.x ) );
	float e = sampleHeightmap( uv + vec2( -texelSize.y, 0.0f ) );
	float w = sampleHeightmap( uv + vec2( texelSize.y, 0.0f ) );
	vec3 sn = vec3( 0.0f , s - n, -( texelSize.y * 2.0f ) );
	vec3 ew = vec3( -( texelSize.x * 2.0f ), e - w, 0.0f );
	sn *= ( texelSize.y * 2.0f );
    ew *= ( texelSize.x * 2.0f );
    sn = normalize( sn );
    ew = normalize( ew );
    vec3 result = normalize( cross( sn, ew ) );
	return result;
}

void cdlod_vertex() {
	// calculate position on the heightmap for height value lookup
	vec3 vertex = getTileVertexPos( position );

	// Pre-sample height to be able to precisely calculate morphing value.
	vec2 preUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( preUV ) * u_height_factor;
	float eyeDistance = distance( vertex, u_camera_position );

	vert_out.morphLerpK = 1.0f - clamp( g_morphConsts.z - eyeDistance * g_morphConsts.w, 0.0f, 1.0f );
	vertex.xz = morphVertex( position, vertex.xz, vert_out.morphLerpK );

	vert_out.heightmapUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( vert_out.heightmapUV ) * u_height_factor;

	// calculate position in world coordinates with the formula:
	// world position = tileOffset + tileVertexPosition * cellsize

	vert_out.world_pos = vec4( vertex, 1.0f );
	vert_out.position = u_view_projection_matrix * vec4( vertex, 1.0f );

	vec3 normal = calculateNormal( vert_out.heightmapUV );
	vert_out.normal = normalize( normal * g_tileScale.xyz );
	vert_out.eyeDir = vec4( vert_out.position.xyz - u_camera_position, eyeDistance );
	//vert_out.lightFactor = clamp( dot( normal, g_diffuseLightDir ), 0.0f, 1.0f );
}

void phong_lighting() {
	//const vec3 surface_normal = normalize( u_normal_matrix * vert_out.normal );
	const vec3 surface_normal = vert_out.normal;
	//const vec4 view_space_coords = u_model_view_matrix * vert_out.world_pos;
	const vec4 view_space_coords = vert_out.position;
	const vec3 ambient = light.la * material.ka;
	vec3 dir_to_light;
	// 0 = directional light
	if( abs(light.position.w) < 0.01f )
		dir_to_light = normalize( light.position.xyz );
	else
		dir_to_light = normalize( light.position.xyz - view_space_coords.xyz );
	float sDotN = max( dot( dir_to_light, surface_normal ), 0.0f );
	vec3 diffuse = light.ld * material.kd * sDotN;
	vec3 spec = vec3(0.0f);
	if( sDotN > 0.0f ) {
		vec3 to_viewer = normalize(-view_space_coords.xyz);
		vec3 reflection = reflect( -dir_to_light, surface_normal );
		spec = light.ls * material.ks * pow( max( dot( reflection, to_viewer ), 0.0f ), material.shininess );
	}
	vert_out.light_intensity = ambient + diffuse + spec;
	//gl_Position = MVP * vec4(VertexPosition,1.0f);
}

void main() {
	cdlod_vertex();
	phong_lighting();
	gl_Position = vert_out.position;
}
