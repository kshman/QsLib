#pragma once

#include <qs_ctn.h>

// 스터브
QN_DECL_LIST(qgListEvent, QgEvent);

struct StubBase
{
	bool				inited;

	void*				oshandle;

	QgFlag				flags;
	QgStubStat			sttis;
	uint				delay;
	int					padding[1];

	QnTimer*			timer;
	double				fps;
	double				run;
	double				active;

	float				refadv;
	float				advance;

	QnRect				bound;
	QnSize				size;

	QgUimKey			key;
	QgUimMouse			mouse;

	qgListEvent			events;
};

struct StubParam
{
	const char*		title;
	int				width;
	int				height;
	int				flags;
};

extern struct StubBase* qg_stub_instance;

extern struct StubBase* stub_system_open(struct StubParam* param);
extern void stub_system_finalize(void);
extern bool stub_system_poll(void);

extern bool stub_internal_mouse_clicks(QimButton button, QimTrack track);


// 렌더 디바이스
extern QgRdh* qg_rdh_instance;

extern void rdh_internal_dispose(QmGam* g);
extern void rdh_internal_reset(QgRdh* self);

extern QgRdh* es2_allocator(void* oshandle, int flags);

#define rdh_caps(rdh)			(qm_cast(rdh, QgRdh)->caps)
#define rdh_tm(rdh)				(qm_cast(rdh, QgRdh)->tm)
#define rdh_param(rdh)			(qm_cast(rdh, QgRdh)->tm)
#define rdh_invokes(rdh)		(qm_cast(rdh, QgRdh)->invokes)

#define rdh_set_flush(rdh,v)	(qm_cast(rdh, QgRdh)->invokes.fluash=(v))
#define rdh_inc_ends(rdh)		(qm_cast(rdh, QgRdh)->invokes.ends++)


// SDL
extern QikKey sdlk_to_qik(uint32_t sdlk);
extern QikMask sdl_kmod_to_qikm(int modifier);
