
#pragma once

#include "terrain/gridmesh.h"
#include "renderer/shader_program.h"
#include "base/camera.h"
#include "omath/mat4.h"
#include <string.h>

static GLuint program = 0;
static gridmesh_t *gridmesh = NULL;

bool mesh_test_create() {
	gridmesh = gridmesh_create( 32, gridmesh );
	if( NULL == gridmesh )
		return false;
	if( !sp_create(
			"src/applications/mesh_test/mesh_test.vert.glsl",
			"src/applications/mesh_test/mesh_test.frag.glsl", &program ) ) {
		gridmesh = gridmesh_delete(gridmesh);
		return false;
	}
	vec3f position = { 0.0f, 1.0f, -1.0f }, target = { 1.0f, 0.0f, 0.0f };
	camera_set_position_and_target( &position, &target );
	camera_set_near_far_plane( 0.1f, 10.0f );
	camera_set_movement_speed(0.3f);
	glUseProgram(program);
	// identity
	const mat4f model_m = { .data = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } };
	sp_set_model_matrix( &model_m );
	return true;
}

void mesh_test_render() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glBindVertexArray(gridmesh->vertex_array);
	glUseProgram(program);
	const vec3f posf = {
			(float)camera_get_position()->x,(float)camera_get_position()->y,(float)camera_get_position()->z
	};
	sp_set_camera_position( &posf );
	sp_set_view_projection_matrix( camera_get_view_projection_matrix() );
	//glDrawElements( window_get_draw_mode(), gridmesh->num_indices, GL_UNSIGNED_INT, NULL );
	glDrawElements( GL_LINES, gridmesh->num_indices, GL_UNSIGNED_INT, NULL );
}

void mesh_test_delete() {
	gridmesh = gridmesh_delete(gridmesh);
	sp_delete(program);
}
