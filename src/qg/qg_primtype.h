#pragma once

// 평면 정점 형식
typedef struct ORTHOVERTEX
{
	QmFloat3		pos;
	QmFloat2		coord;
	QmKolor			color;
} OrthoVertex;

// 평면 정점 설정
INLINE void QM_VECTORCALL ortho_vertex_set(OrthoVertex* o, const QMVEC pos, QmFloat2 coord, const QmKolor color)
{
	qm_vec_to_float3(pos, &o->pos);
	o->coord = coord;
	o->color = color;
}

// 평면 정점 설정2
INLINE void QM_VECTORCALL ortho_vertex_set_param(OrthoVertex* o, float x, float y, float z, float u, float v, const QmKolor color)
{
	o->pos = (QmFloat3){ x, y, z };
	o->coord = (QmFloat2){ u, v };
	o->color = color;
}

// 3D 정점 형식
typedef struct FRSTVERTEX
{
	QmFloat3		pos;
	QmFloat3		normal;
	QmFloat2		coord;
	QmKolor			color;
} FrstVertex;

// 3D 정점 설정
INLINE void QM_VECTORCALL frst_vertex_set(FrstVertex* f, const QMVEC pos, const QMVEC normal, QmFloat2 coord, const QmKolor color)
{
	qm_vec_to_float3(pos, &f->pos);
	qm_vec_to_float3(normal, &f->normal);
	f->coord = coord;
	f->color = color;
}

// 3D 정점 설정2
INLINE void QM_VECTORCALL frst_vertex_set_param(FrstVertex* f, float x, float y, float z, float nx, float ny, float nz, float u, float v, const QmKolor color)
{
	f->pos = (QmFloat3){ x, y, z };
	f->normal = (QmFloat3){ nx, ny, nz };
	f->coord = (QmFloat2){ u, v };
	f->color = color;
}

