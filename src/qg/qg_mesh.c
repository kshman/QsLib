//
// qg_mesh.c - 메시
// 2024-2-22 by kim
//

#include "pch.h"
#include "qg_dpct.h"
#include "qg_stub.h"
#include "qs_supp.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 6011)
#endif

#define PAR_SHAPES_IMPLEMENTATION
#define PAR_MALLOC(T, N)		qn_alloc(N, T)
#define PAR_CALLOC(T, N)		qn_alloc_zero(N, T)
#define PAR_REALLOC(T, BUF, N)	qn_realloc(BUF, N, T)
#define PAR_FREE(BUF)			qn_free(BUF)
#define sqrtf					qm_sqrtf
#define printf					qn_outputf
#include "par/par_shapes.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif


////////////////////////////////////////////////////////////////////////
// 메시

#undef VAR_CHK_NAME
#define VAR_CHK_NAME	"Mesh"

// 메시 그리기
static void _mesh_draw(QnGam g)
{
	QgMesh* self = qn_cast_type(g, QgMesh);
	size_t i;

	qg_set_world(&self->base.trfm.mcalc.s);

	for (i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (self->vbuffers[i] != NULL)
			qg_set_vertex(i, self->vbuffers[i]);
	}
	if (self->ibuffer == NULL)
		qg_draw(QGTPG_TRI, self->mesh.vertices);
	else
	{
		qg_set_index(self->ibuffer);
		qg_draw_indexed(QGTPG_TRI, self->mesh.polygons * 3);
	}
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
		PAR_SHAPES_T p0 = mesh->triangles[i] * 3 + 0;
		PAR_SHAPES_T p1 = mesh->triangles[i] * 3 + 1;
		PAR_SHAPES_T p2 = mesh->triangles[i] * 3 + 2;
		pos[i] = (QmFloat3){ mesh->points[p0], mesh->points[p1], mesh->points[p2] };
		norm[i] = (QmFloat3){ mesh->normals[p0], mesh->normals[p1], mesh->normals[p2] };
		PAR_SHAPES_T t0 = mesh->triangles[i] * 2 + 0;
		PAR_SHAPES_T t1 = mesh->triangles[i] * 2 + 1;
		coord[i] = (QmFloat2){ mesh->tcoords[t0], mesh->tcoords[t1] };
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
		PAR_SHAPES_T p0 = mesh->triangles[i] * 3 + 0;
		PAR_SHAPES_T p1 = mesh->triangles[i] * 3 + 1;
		PAR_SHAPES_T p2 = mesh->triangles[i] * 3 + 2;
		pos[i] = (QmFloat3){ mesh->points[p0], mesh->points[p1], mesh->points[p2] };
		norm[i] = (QmFloat3){ mesh->normals[p0], mesh->normals[p1], mesh->normals[p2] };
		PAR_SHAPES_T t0 = mesh->triangles[i] * 2 + 0;
		PAR_SHAPES_T t1 = mesh->triangles[i] * 2 + 1;
		coord[i] = (QmFloat2){ mesh->tcoords[t0], mesh->tcoords[t1] };
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
	static byte lo_sizes[QGLOT_MAX_VALUE] =
	{
		/* UNKNOWN */ 0,
		/* FLOAT   */ 4 * sizeof(float), 3 * sizeof(float), 2 * sizeof(float), 1 * sizeof(float),
		/* INT     */ 4 * sizeof(int), 3 * sizeof(int), 2 * sizeof(int), 1 * sizeof(int),
		/* HALF-F  */ 4 * sizeof(half_t), 2 * sizeof(half_t), 1 * sizeof(half_t), 1 * sizeof(int),
		/* HALF-I  */ 4 * sizeof(short), 2 * sizeof(short), 1 * sizeof(short), 1 * sizeof(int),
		/* BYTE    */ 4 * sizeof(byte), 3 * sizeof(byte), 2 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte), 1 * sizeof(byte), 2 * sizeof(ushort),
		/* USHORT  */ 2 * sizeof(ushort), 2 * sizeof(ushort), 2 * sizeof(ushort),
	};
	static byte lu_formats[QGLOU_MAX_VALUE] =
	{
		/* QGLOU_UNKNOWN,		*/	0,
		/* QGLOU_POSITION,		*/	QGLOT_FLOAT3,
		/* QGLOU_COORD1,		*/	QGLOT_FLOAT2,
		/* QGLOU_COORD2,		*/	QGLOT_FLOAT2,
		/* QGLOU_COORD3,		*/	QGLOT_FLOAT2,
		/* QGLOU_COORD4,		*/	QGLOT_FLOAT2,
		/* QGLOU_NORMAL1,		*/	QGLOT_FLOAT3,
		/* QGLOU_NORMAL2,		*/	QGLOT_FLOAT3,
		/* QGLOU_NORMAL3,		*/	QGLOT_FLOAT3,
		/* QGLOU_NORMAL4,		*/	QGLOT_FLOAT3,
		/* QGLOU_BINORMAL1,		*/	QGLOT_FLOAT3,
		/* QGLOU_BINORMAL2,		*/	QGLOT_FLOAT3,
		/* QGLOU_TANGENT1,		*/	QGLOT_FLOAT3,
		/* QGLOU_TANGENT2,		*/	QGLOT_FLOAT3,
		/* QGLOU_COLOR1,		*/	QGLOT_BYTE4,
		/* QGLOU_COLOR2,		*/	QGLOT_BYTE4,
		/* QGLOU_COLOR3,		*/	QGLOT_BYTE4,
		/* QGLOU_COLOR4,		*/	QGLOT_BYTE4,
		/* QGLOU_COLOR5,		*/	QGLOT_BYTE4,
		/* QGLOU_COLOR6,		*/	QGLOT_BYTE4,
		/* QGLOU_BLEND_WEIGHT,	*/	0,
		/* QGLOU_BLEND_INDEX,	*/	0,
		/* QGLOU_BLEND_EXTRA,	*/	0,
	};

	VAR_CHK_IF_ZERO(self->layout.count, false);
	VAR_CHK_IF_ZERO(self->mesh.vertices, false);
	VAR_CHK_IF_ZERO(self->mesh.polygons, false);
	VAR_CHK_IF_NULL(self->mesh.position, false);

	size_t i, z, n;
	ushort loac[QGLOS_MAX_VALUE] = { 0, }, losz[QGLOS_MAX_VALUE] = { 0, };
	QgLayoutInput* stages[QGLOS_MAX_VALUE][QGLOU_MAX_SIZE] = { {NULL,}, };
	for (i = 0; i < self->layout.count; i++)
	{
		QgLayoutInput* input = &self->layout.inputs[i];
		if ((size_t)input->stage >= QGLOS_MAX_VALUE)
		{
			qn_mesgfb(VAR_CHK_NAME, "invalid layout stage: %d", input->stage);
			return false;
		}
		if ((size_t)input->format >= QGLOT_MAX_VALUE)
		{
			qn_mesgfb(VAR_CHK_NAME, "invalid layout format: %d", input->format);
			return false;
		}
		if ((size_t)input->usage >= QGLOU_MAX_VALUE)
		{
			qn_mesgfb(VAR_CHK_NAME, "invalid layout usage: %d", input->usage);
			return false;
		}
		byte format = lu_formats[input->usage];
		if (format == 0)
		{
			qn_mesgfb(VAR_CHK_NAME, "layout not support: %d", input->usage);
			return false;
		}
		if (input->format != format)
		{
			qn_mesgfb(VAR_CHK_NAME, "layout format not match: %d (need: %d)", input->format, format);
			return false;
		}

		QgLayoutStage stage = input->stage;
		input->normalized = losz[stage];
		stages[input->stage][loac[stage]] = input;
		loac[stage]++;
		losz[stage] += lo_sizes[input->format];
	}

	byte* vdata[QGLOS_MAX_VALUE];
	for (i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		qn_unload(self->vbuffers[i]);
		if (loac[i] == 0)
		{
			self->vbuffers[i] = NULL;
			vdata[i] = NULL;
			continue;
		}
		self->vbuffers[i] = qg_create_buffer(QGBUFFER_VERTEX, self->mesh.vertices, losz[i], NULL);
		vdata[i] = qg_buffer_map(self->vbuffers[i], true);
	}

	size_t vertices = (size_t)self->mesh.vertices;
	for (z = 0; z < QGLOS_MAX_VALUE; z++)
	{
		size_t stride = losz[z];
		for (i = 0; i < loac[z]; i++)
		{
			QgLayoutInput* input = stages[z][i];
			byte* ptr = vdata[z] + input->normalized;
#define CASE_SET_VALUE(TYPE, DATA, DEF)\
	if (DATA == NULL)\
	{\
		for (n = 0; n < vertices; n++)\
		{\
			*(TYPE*)ptr = DEF;\
			ptr += stride;\
		}\
	} else {\
		for (n = 0; n < vertices; n++)\
		{\
			*(TYPE*)ptr = DATA[n];\
			ptr += stride;\
		}\
	}
#define CASE_FILL_VALUE(TYPE, DEF)\
	for (n = 0; n < vertices; n++)\
	{\
		*(TYPE*)ptr = DEF;\
		ptr += stride;\
	}
			switch (input->usage)
			{
				case QGLOU_POSITION:
					CASE_SET_VALUE(QmFloat3, self->mesh.position, ((QmFloat3) { 0.0f, 0.0f, 0.0f }));
					break;
				case QGLOU_COORD1:
					CASE_SET_VALUE(QmFloat2, self->mesh.coord[0], ((QmFloat2) { 0.0f, 0.0f }));
					break;
				case QGLOU_COORD2:
					CASE_SET_VALUE(QmFloat2, self->mesh.coord[1], ((QmFloat2) { 0.0f, 0.0f }));
					break;
				case QGLOU_COORD3:
				case QGLOU_COORD4:
					CASE_FILL_VALUE(QmFloat2, ((QmFloat2) { 0.0f, 0.0f }));
					break;
				case QGLOU_NORMAL1:
					CASE_SET_VALUE(QmFloat3, self->mesh.normal[0], ((QmFloat3) { 0.0f, 1.0f, 0.0f }));
					break;
				case QGLOU_NORMAL2:
					CASE_SET_VALUE(QmFloat3, self->mesh.normal[1], ((QmFloat3) { 0.0f, 1.0f, 0.0f }));
					break;
				case QGLOU_NORMAL3:
				case QGLOU_NORMAL4:
					CASE_FILL_VALUE(QmFloat3, ((QmFloat3) { 0.0f, 1.0f, 0.0f }));
					break;
				case QGLOU_BINORMAL1:
					CASE_SET_VALUE(QmFloat3, self->mesh.binormal, ((QmFloat3) { 0.0f, 1.0f, 0.0f }));
					break;
				case QGLOU_BINORMAL2:
					CASE_FILL_VALUE(QmFloat3, ((QmFloat3) { 0.0f, 1.0f, 0.0f }));
					break;
				case QGLOU_TANGENT1:
					CASE_SET_VALUE(QmFloat3, self->mesh.tangent, ((QmFloat3) { 0.0f, 1.0f, 0.0f }));
					break;
				case QGLOU_TANGENT2:
					CASE_FILL_VALUE(QmFloat3, ((QmFloat3) { 0.0f, 1.0f, 0.0f }));
					break;
				case QGLOU_COLOR1:
					CASE_SET_VALUE(uint, self->mesh.color[0], 0xFFFFFFFF);
					break;
				case QGLOU_COLOR2:
					CASE_SET_VALUE(uint, self->mesh.color[1], 0xFFFFFFFF);
					break;
				case QGLOU_COLOR3:
				case QGLOU_COLOR4:
				case QGLOU_COLOR5:
				case QGLOU_COLOR6:
					CASE_FILL_VALUE(uint, 0xFFFFFFFF);
					break;
			}
#undef CASE_SET_VALUE
#undef CASE_FILL_VALUE
		}
	}

	for (i = 0; i < QGLOS_MAX_VALUE; i++)
	{
		if (vdata[i] != NULL)
			qg_buffer_unmap(self->vbuffers[i]);
	}

	if (self->mesh.index != NULL)
	{
		qn_unload(self->ibuffer);
		self->ibuffer = qg_create_buffer(QGBUFFER_INDEX, self->mesh.polygons * 3, sizeof(int), self->mesh.index);
	}

	return true;
}
