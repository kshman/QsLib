#pragma once

// 평면 정점 형식
typedef struct ORTHOVERTEX
{
	QmFloat4		postex;
	uint			color;
} OrthoVertex;

// 평면 정점 설정
INLINE void QM_VECTORCALL ortho_vertex_set(OrthoVertex* o, const QmVec postex, const QmVec color)
{
	o->postex = *(QmFloat4*)&postex.v;
	o->color = qm_color_to_uint(color);
}

// 평면 정점 설정2
INLINE void QM_VECTORCALL ortho_vertex_set_param(OrthoVertex* o, float x, float y, float u, float v, const QmVec color)
{
	o->postex = (QmFloat4){ x, y, u, v };
	o->color = qm_color_to_uint(color);
}

// 3D 정점 형식
typedef struct FRSTVERTEX
{
	QmFloat3		pos;
	QmFloat3		normal;
	QmFloat2		tex;
	QmFloat4		color;
} FrstVertex;
