#pragma once

// 전역
extern size_t qg_number(void);

// 스터브
struct stubParam
{
	const char*		title;
	int				width;
	int				height;
	int				flags;
};

extern qgStub* _stub_allocator();
extern void _stub_dispose(qgStub* g);
extern void _stub_construct(qgStub* g, struct stubParam* param);
extern void _stub_finalize(qgStub* g);

extern bool _stub_mouse_clicks(qgStub* g, qImButton button, qimTrack track);

// 렌더 디바이스
extern qgRdh* _es2_allocator();
extern void _rdh_dispose(qgRdh* g);

#define qg_rdh_caps(rdh)		(&qm_cast(rdh, qgRdh)->caps)
#define qg_rdh_tm(rdh)			(&qm_cast(rdh, qgRdh)->tm)
#define qg_rdh_param(rdh)		(&qm_cast(rdh, qgRdh)->tm)
#define qg_rdh_invokes(rdh)		(&qm_cast(rdh, qgRdh)->invokes)
