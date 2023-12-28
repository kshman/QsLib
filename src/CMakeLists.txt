# CMakeList.txt: qs에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#

# 설정
if (MSVC)
	add_definitions (-DUSE_D12 -DUSE_ES3)
	if (CMAKE_BUILD_TYPE MATCHES Release)
		add_compile_options (/sdl- /GS- /Gy /FAsu /Fa)
		add_link_options (/GL)
	endif()
	add_compile_options(/arch:AVX2 /fp:fast)
elseif (EMSCRIPTEN)
	add_definitions (-DUSE_ES3)
	add_link_options (-sASYNCIFY)
	target_link_libraries (qs PUBLIC m)
else()
	add_definitions (-DUSE_ES3 -DUSE_WAYLAND)
	target_link_libraries (qs PUBLIC m)
endif()

if (NOT EMSCRIPTEN AND CLANG AND CMAKE_BUILD_TYPE MATCHES Release)
	add_compile_options (-flto)
	add_link_options (-flto)
endif()

# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_library (qs SHARED "pch.c" "pch.h" "qs_conf.h" "qn/qn.c" "qn/qn.c" "qn/qn_debug.c" "qn/qn_file.c" "qn/qn_hash.c" "qn/qn_mem.c" "qn/qn_mlu.c" "qn/qn_sort.c" "qn/qn_str.c" "qn/qn_thd.c" "qn/qn_time.c" "qn/qm_math.c" "qn/qs_gam.c" "qn/zlib/adler32.c" "qn/zlib/compress.c" "qn/zlib/crc32.c" "qn/zlib/deflate.c" "qn/zlib/gzclose.c" "qn/zlib/gzlib.c" "qn/zlib/gzread.c" "qn/zlib/gzwrite.c" "qn/zlib/infback.c" "qn/zlib/inffast.c" "qn/zlib/inflate.c" "qn/zlib/inftrees.c" "qn/zlib/trees.c" "qn/zlib/uncompr.c" "qn/zlib/zutil.c" "qg/qg_kmc.c" "qn/PatrickPowell_snprintf.c" "qg/stub/qg_stub.c" "qg/stub/qgwin_stub.h" "qg/stub/qgwin_stub.c" "qg/stub/qgwin_stub_func.h" "qg/stub/qgwlx_stub.h" "qg/stub/qgwlx_stub.c" "qg/stub/qgwlx_stub_func.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET qs PROPERTY C_STANDARD 17)
endif()
