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
extern void _stub_construct(pointer_t g, struct stubParam* param);
extern void _stub_finalize(pointer_t g);

extern bool _stub_mouse_clicks(pointer_t g, qImButton button, qimTrack track);

// 렌더 디바이스
extern bool _rdh_on_init(pointer_t g, const char* title, int width, int height, int flags);
extern void _rdh_on_disp(pointer_t g);
extern void _rdh_reset(pointer_t g);

extern bool _rdh_impl_begin(pointer_t g);
extern void _rdh_impl_end(pointer_t g);
extern void _rdh_impl_primitive_begin(pointer_t g, qgTopology tpg, int count, int stride, pointer_t* ptr);
extern void _rdh_impl_primitive_end(pointer_t g);
extern void _rdh_impl_indexed_primitive_begin(pointer_t g, qgTopology tpg, int vcount, int vstride, pointer_t* vptr, int icount, int istride, pointer_t* iptr);
extern void _rdh_impl_indexed_primitive_end(pointer_t g);
