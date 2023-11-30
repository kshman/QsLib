#pragma once

// 스터브
struct stubParam
{
	const char*		title;
	int				width;
	int				height;
	int				flags;
};

extern pointer_t _stub_allocator();
extern void _stub_dispose(pointer_t g);
extern void _stub_construct(pointer_t g, struct stubParam* param);
extern void _stub_finalize(pointer_t g);

extern bool _stub_mouse_clicks(pointer_t g, qImButton button, qimTrack track);

// 렌더 디바이스
extern pointer_t _es2_allocator();
extern void _rdh_construct(pointer_t g, qgStub* stub);
extern void _rdh_dispose(pointer_t g);
extern void _rdh_finalize(pointer_t g);
extern void _rdh_reset(pointer_t g);

