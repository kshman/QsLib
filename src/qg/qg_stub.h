#pragma once

#include <qs_ctn.h>

// 스터브
QN_DECL_LIST(qgListEvent, QgEvent);

struct StubBase
{
	bool				inited;

	QgFlag				flags;
	QgStubStat			sttis;
	uint				delay;
	int					_padding[1];

	QnTimer*			timer;
	double				fps;
	double				run;
	double				active;

	float				refadv;
	float				advance;

	QnRect				bound;
	QnPoint				size;

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
extern QgRdh* _es2_allocator();
extern void _rdh_dispose(QgRdh* g);

#define qg_rdh_caps(rdh)		(&qm_cast(rdh, QgRdh)->caps)
#define qg_rdh_tm(rdh)			(&qm_cast(rdh, QgRdh)->tm)
#define qg_rdh_param(rdh)		(&qm_cast(rdh, QgRdh)->tm)
#define qg_rdh_invokes(rdh)		(&qm_cast(rdh, QgRdh)->invokes)

// SDL
extern QikKey _sdlk_to_qik(uint32_t sdlk);
extern QikMask _sdl_kmod_to_qikm(int modifier);
