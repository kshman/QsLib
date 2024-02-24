//
// qg_mesh.c - 메시
// 2024-2-22 by kim
//

#include "pch.h"
#include "qg_dpct.h"
#include "qg_stub.h"
#include "qs_supp.h"

#define PAR_SHAPES_IMPLEMENTATION
#define PAR_MALLOC(T, N)		qn_alloc(N, T)
#define PAR_CALLOC(T, N)		qn_alloc_zero(N, T)
#define PAR_REALLOC(T, BUF, N)	qn_realloc(BUF, N, T)
#define PAR_FREE(BUF)			qn_free(BUF)
#define sqrtf					qm_sqrtf
#include "par/par_shapes.h"


////////////////////////////////////////////////////////////////////////
// 메시

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Mesh"

// 메시 그리기
static void _mesh_draw(QnGam g)
{
	QgMesh* self = qn_cast_type(g, QgMesh);
	(void)self;
}

// 메시 제거
static void _mesh_dispose(QnGam g)
{
	QgMesh* self = qn_cast_type(g, QgMesh);

	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
		qn_unload(self->vbuffers[i]);
	qn_unload(self->ibuffer);

	if (self->layout.count)
		qn_free(self->layout.inputs);

	if (self->mesh.vertices > 0)
	{
		qn_free(self->mesh.position);
		qn_free(self->mesh.coord[0]);
		qn_free(self->mesh.coord[1]);
		qn_free(self->mesh.normal[0]);
		qn_free(self->mesh.normal[1]);
		qn_free(self->mesh.binormal);
		qn_free(self->mesh.tangent);
		qn_free(self->mesh.color[0]);
		qn_free(self->mesh.color[1]);
	}
	qn_free(self->mesh.index);

	qn_free(self);
}

//
QgMesh* qg_create_mesh(const char* name)
{
	QgMesh* self = qn_alloc_zero_1(QgMesh);
	_dpct_init(qn_cast_type(self, QgDpct), name, NULL);

	static const QN_DECL_VTABLE(QGDPCT) vt_qg_mesh =
	{
		{
			VAR_CHK_NAME,
			_mesh_dispose,
		},
		_dpct_update,
		_mesh_draw,
		_dpct_set_loc,
		_dpct_set_rot,
		_dpct_set_scl,
	};
	return qn_gam_init(self, vt_qg_mesh);
}

//
void qg_mesh_set_layout(QgMesh* self, const QgLayoutData* lo)
{
	if (self->layout.count)
	{
		self->layout.count = 0;
		qn_free(self->layout.inputs);
	}

	if (lo != NULL)
	{
		self->layout.count = lo->count;
		self->layout.inputs = qn_alloc(lo->count, QgLayoutInput);
		memcpy(self->layout.inputs, lo->inputs, sizeof(QgLayoutInput) * lo->count);
	}
}

//
bool qg_mesh_gen_cube(QgMesh* self, float width, float height, float depth)
{
	par_shapes_mesh* mesh = par_shapes_create_cube();
	mesh->tcoords = PAR_MALLOC(float, mesh->npoints * 2);
	for (int i = 0; i < 2 * mesh->npoints; i++)
		mesh->tcoords[i] = 0.0f;
	par_shapes_scale(mesh, width, height, depth);
	par_shapes_translate(mesh, -width * 0.5f, 0.0f, -depth * 0.5f);
	par_shapes_compute_normals(mesh);

	int vertex_count = 3 * mesh->ntriangles;
	QmFloat3* pos = self->mesh.position = qn_realloc(self->mesh.position, vertex_count, QmFloat3);
	QmFloat3* norm = self->mesh.normal[0] = qn_realloc(self->mesh.normal[0], vertex_count, QmFloat3);
	QmFloat2* coord = self->mesh.coord[0] = qn_realloc(self->mesh.coord[0], vertex_count, QmFloat2);

	self->mesh.vertices = vertex_count;
	self->mesh.polygons = mesh->ntriangles;

	for (int i = 0; i < vertex_count; i++)
	{
		pos[i] = (QmFloat3){ mesh->points[i * 3 + 0], mesh->points[i * 3 + 1], mesh->points[i * 3 + 2] };
		norm[i] = (QmFloat3){ mesh->normals[i * 3 + 0], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2] };
		coord[i] = (QmFloat2){ mesh->tcoords[i * 2 + 0], mesh->tcoords[i * 2 + 1] };
	}

	par_shapes_free_mesh(mesh);

	return true;
}

//
bool qg_mesh_gen_plane(QgMesh* self, float width, float depth, int split_width, int split_depth)
{
	qn_return_when_fail(split_width >= 1 && split_depth >= 1, false);

	par_shapes_mesh* mesh = par_shapes_create_plane(split_width, split_depth);
	par_shapes_scale(mesh, width, depth, 1.0f);
	par_shapes_rotate(mesh, -QM_PI_H, (float[]) { 1.0f, 0.0f, 0.0f });
	par_shapes_translate(mesh, -width / 2.0f, 0.0f, depth / 2.0f);

	int vertex_count = mesh->ntriangles * 3;
	QmFloat3* pos = self->mesh.position = qn_realloc(self->mesh.position, vertex_count, QmFloat3);
	QmFloat3* norm = self->mesh.normal[0] = qn_realloc(self->mesh.normal[0], vertex_count, QmFloat3);
	QmFloat2* coord = self->mesh.coord[0] = qn_realloc(self->mesh.coord[0], vertex_count, QmFloat2);

	self->mesh.vertices = vertex_count;
	self->mesh.polygons = mesh->ntriangles;

	for (int i = 0; i < vertex_count; i++)
	{
		pos[i] = (QmFloat3){ mesh->points[i * 3 + 0], mesh->points[i * 3 + 1], mesh->points[i * 3 + 2] };
		norm[i] = (QmFloat3){ mesh->normals[i * 3 + 0], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2] };
		coord[i] = (QmFloat2){ mesh->tcoords[i * 2 + 0], mesh->tcoords[i * 2 + 1] };
	}

	par_shapes_free_mesh(mesh);

	return true;
}

//
bool qg_mesh_gen_poly(QgMesh* self, int sides, float radius)
{
	qn_return_when_fail(sides >= 3, false);

	int i, vertex_count = sides * 3;
	float f = 0.0f, step = QM_TAU / sides;

	QmFloat3* pos = self->mesh.position = qn_realloc(self->mesh.position, vertex_count, QmFloat3);
	for (i = 0; i < vertex_count - 2; i += 3)
	{
		float s1, c1, s2, c2;
		qm_sincosf(f * radius, &s1, &c1);
		qm_sincosf((f + step) * radius, &s2, &c2);
		pos[i + 0] = (QmFloat3){ 0.0f, 0.0f, 0.0f };
		pos[i + 1] = (QmFloat3){ c1, 0.0f, s1 };
		pos[i + 2] = (QmFloat3){ c2, 0.0f, s2 };
		f += step;
	}

	QmFloat3* norm = self->mesh.normal[0] = qn_realloc(self->mesh.normal[0], vertex_count, QmFloat3);
	for (i = 0; i < vertex_count; i++)
		norm[i] = (QmFloat3){ 0.0f, 1.0f, 0.0f };

	QmFloat2* coord = self->mesh.coord[0] = qn_realloc(self->mesh.coord[0], vertex_count, QmFloat2);
	for (i = 0; i < vertex_count; i++)
		coord[i] = (QmFloat2){ 0.0f, 0.0f };

	self->mesh.vertices = vertex_count;
	self->mesh.polygons = sides;

	return true;
}

//
bool qg_mesh_gen_sphere(QgMesh* self, float radius, int slices, int stacks)
{
	qn_return_when_fail(slices >= 3 && stacks >= 2, false);

	par_shapes_mesh* mesh = par_shapes_create_parametric_sphere(slices, stacks);
	par_shapes_scale(mesh, radius, radius, radius);
	par_shapes_compute_normals(mesh);

	int vertex_count = 3 * mesh->ntriangles;
	QmFloat3* pos = self->mesh.position = qn_realloc(self->mesh.position, vertex_count, QmFloat3);
	QmFloat3* norm = self->mesh.normal[0] = qn_realloc(self->mesh.normal[0], vertex_count, QmFloat3);
	QmFloat2* coord = self->mesh.coord[0] = qn_realloc(self->mesh.coord[0], vertex_count, QmFloat2);

	self->mesh.vertices = vertex_count;
	self->mesh.polygons = mesh->ntriangles;

	for (int i = 0; i < vertex_count; i++)
	{
		pos[i] = (QmFloat3){ mesh->points[i * 3 + 0], mesh->points[i * 3 + 1], mesh->points[i * 3 + 2] };
		norm[i] = (QmFloat3){ mesh->normals[i * 3 + 0], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2] };
		coord[i] = (QmFloat2){ mesh->tcoords[i * 2 + 0], mesh->tcoords[i * 2 + 1] };
	}

	par_shapes_free_mesh(mesh);

	return true;
}

//
bool qg_mesh_gen_torus(QgMesh* self, float radius, float size, int segment, int sides)
{
	qn_return_when_fail(segment >= 3 && sides >= 3, false);

	par_shapes_mesh* mesh = par_shapes_create_torus(segment, sides, radius);
	par_shapes_scale(mesh, size, size, size);
	par_shapes_compute_normals(mesh);

	int vertex_count = 3 * mesh->ntriangles;
	QmFloat3* pos = self->mesh.position = qn_realloc(self->mesh.position, vertex_count, QmFloat3);
	QmFloat3* norm = self->mesh.normal[0] = qn_realloc(self->mesh.normal[0], vertex_count, QmFloat3);
	QmFloat2* coord = self->mesh.coord[0] = qn_realloc(self->mesh.coord[0], vertex_count, QmFloat2);

	self->mesh.vertices = vertex_count;
	self->mesh.polygons = mesh->ntriangles;

	for (int i = 0; i < vertex_count; i++)
	{
		pos[i] = (QmFloat3){ mesh->points[i * 3 + 0], mesh->points[i * 3 + 1], mesh->points[i * 3 + 2] };
		norm[i] = (QmFloat3){ mesh->normals[i * 3 + 0], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2] };
		coord[i] = (QmFloat2){ mesh->tcoords[i * 2 + 0], mesh->tcoords[i * 2 + 1] };
	}

	par_shapes_free_mesh(mesh);

	return true;
}

//
bool qg_mesh_build(QgMesh* self)
{
	VAR_CHK_IF_ZERO(self->layout.count, false);
	VAR_CHK_IF_ZERO(self->mesh.vertices, false);
	VAR_CHK_IF_ZERO(self->mesh.polygons, false);

	/*
	// 버텍스 버퍼 생성
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (self->vbuffers[i] == NULL)
			self->vbuffers[i] = qn_create_vbuffer();
	}

	// 인덱스 버퍼 생성
	if (self->ibuffer == NULL)
		self->ibuffer = qn_create_ibuffer();

	// 버텍스 버퍼에 데이터 쓰기
	for (int i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (self->vbuffers[i] != NULL)
		{
			qn_vbuffer_write(self->vbuffers[i], self->mesh.position, self->mesh.vertices, sizeof(QmFloat3), i);
			qn_vbuffer_write(self->vbuffers[i], self->mesh.coord[i], self->mesh.vertices, sizeof(QmFloat2), i + 1);
			qn_vbuffer_write(self->vbuffers[i], self->mesh.normal[i], self->mesh.vertices, sizeof(QmFloat3), i + 2);
		}
	}

	// 인덱스 버퍼에 데이터 쓰기
	qn_ibuffer_write(self->ibuffer, self->mesh.index, self->mesh.polygons * 3, sizeof(int));
	*/

	return true;
}
