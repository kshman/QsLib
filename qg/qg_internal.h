#pragma once

// 입력
extern qIkKey _sdlk_to_qik(uint32_t sdlk);
extern qIkMask _sdl_kmod_to_qikm(int modifier);

// 스터브
extern bool _stub_poll(pointer_t g);
extern bool _stub_on_init(pointer_t g);
extern void _stub_on_disp(pointer_t g);
extern bool _stub_mouse_clicks(pointer_t g, qImButton button, qimTrack track);
