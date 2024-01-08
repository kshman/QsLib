# CMakeList.txt: qs에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#

# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_library (qs ${QSBUILD_LIBRARY_TYPE} 
	"pch.c" "pch.h" "qs_conf.h" "qn/PatrickPowell_snprintf.c" "qn/qm_math.c" 
	"qn/qn.c" "qn/qn_file.c" "qn/qn_prf.c" "qn/qn_mlu.c" "qn/qn_str.c" "qn/qn_thd.c" "qn/qn_time.c" "qn/qs_gam.c" 
	"qn/zlib/adler32.c" "qn/zlib/compress.c" "qn/zlib/crc32.c" "qn/zlib/deflate.c" "qn/zlib/gzclose.c" "qn/zlib/gzlib.c" "qn/zlib/gzread.c" "qn/zlib/gzwrite.c" "qn/zlib/infback.c" "qn/zlib/inffast.c" "qn/zlib/inflate.c" "qn/zlib/inftrees.c" "qn/zlib/trees.c" "qn/zlib/uncompr.c" "qn/zlib/zutil.c" 
	"qg/qg_kmc.c" "qg/qg_stub.c" "qg/qg_rdh.c" "qg/stub/qgrdh_es.c")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET qs PROPERTY C_STANDARD 17)
endif()

# 설정
if (QSBUILD_SDL2)
	target_compile_definitions(qs PRIVATE USE_SDL2)
	target_sources(qs PRIVATE "qg/stub/qgstub_sdl.c")
	if (MSVC)
		set (MSVC_SDL2_DIRECTORY ../builds/external/SDL2Windows)
		target_compile_definitions(qs PRIVATE USE_D12 USE_ES)
		target_include_directories(qs PRIVATE "${MSVC_SDL2_DIRECTORY}")
		target_link_directories(qs PRIVATE "${MSVC_SDL2_DIRECTORY}")
		if (CMAKE_CL_64)
			target_link_options (qs PRIVATE SDL2x64.lib)
			#copy_file(qs "${MSVC_SDL2_DIRECTORY}/SDL2x64.dll" "${LIBRARY_OUTPUT_PATH}")
		else()
			target_link_options (qs PRIVATE SDL2.lib)
			#copy_file(qs "${MSVC_SDL2_DIRECTORY}/SDL2.dll" "${LIBRARY_OUTPUT_PATH}")
		endif()
	else()
		target_compile_definitions(qs PRIVATE USE_ES)
		target_link_libraries (qs PRIVATE SDL2)
	endif()
endif()
if (QSBUILD_WINDOWS)
	target_sources(qs PRIVATE "qg/stub/qgstub_win.c" "qg/stub/qgstub_win_func.h")
	target_compile_options (qs PRIVATE "/arch:AVX2" "/fp:fast")
	target_compile_definitions(qs PRIVATE USE_D12 USE_ES)
endif()
if (QSBUILD_WAYLAND)
	target_sources(qs PRIVATE "qg/stub/qgstub_wlx.c" "qg/stub/qgstub_wlx_func.h")
	target_compile_definitions(qs PRIVATE USE_WAYLAND USE_ES)
	
	find_program(WAYLAND_SCANNER NAMES wayland-scanner)
	include (FindPkgConfig)
	pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.15)
	pkg_get_variable(WAYLAND_PROTOCOL_BASE wayland-protocols pkgdatadir)
	pkg_get_variable(WAYLAND_CLIENT_BASE wayland-client pkgdatadir)

	macro(wayland_generate protocol_file output_file)
		add_custom_command(OUTPUT "${output_file}.h"
			COMMAND "${WAYLAND_SCANNER}" client-header "${protocol_file}" "${output_file}.h"
			DEPENDS "${protocol_file}"
			VERBATIM)
		add_custom_command(OUTPUT "${output_file}-code.h"
			COMMAND "${WAYLAND_SCANNER}" private-code "${protocol_file}" "${output_file}-code.h"
			DEPENDS "${protocol_file}"
			VERBATIM)
		target_sources(qs PRIVATE "${output_file}.h" "${output_file}-code.h")
	endmacro()

	wayland_generate("${WAYLAND_CLIENT_BASE}/wayland.xml" "${CMAKE_BINARY_DIR}/src/wayland-wayland")
	wayland_generate("${WAYLAND_PROTOCOL_BASE}/stable/xdg-shell/xdg-shell.xml" "${CMAKE_BINARY_DIR}/src/wayland-xdg-shell")
	#wayland_generate("${WAYLAND_PROTOCOL_BASE}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml" "${CMAKE_BINARY_DIR}/src/wayland-xdg-decoration-unstable-v1")
	#wayland_generate("${WAYLAND_PROTOCOL_BASE}/stable/viewporter/viewporter.xml" "${CMAKE_BINARY_DIR}/src/wayland-viewporter")
	#wayland_generate("${WAYLAND_PROTOCOL_BASE}/unstable/relative-pointer/relative-pointer-unstable-v1.xml" "${CMAKE_BINARY_DIR}/src/wayland-relative-pointer-unstable-v1")
	#wayland_generate("${WAYLAND_PROTOCOL_BASE}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml" "${CMAKE_BINARY_DIR}/src/wayland-pointer-constraints-unstable-v1")
	#wayland_generate("${WAYLAND_PROTOCOL_BASE}/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml" "${CMAKE_BINARY_DIR}/src/wayland-idle-inhibit-unstable-v1")

	pkg_check_modules(Wayland REQUIRED wayland-client>=0.2.7 wayland-cursor>=0.2.7 wayland-egl>=0.2.7 xkbcommon>=0.5.0)
	target_include_directories (qs PRIVATE ${Wayland_INCLUDE_DIRS})
endif()
if (QSBUILD_EMSCRIPTEN)
	target_sources(qs PRIVATE "qg/stub/qgstub_emscripten.c")
	target_compile_definitions(qs PRIVATE USE_ES)
	target_link_options (qs PRIVATE "-sASYNCIFY")
endif()

if (CLANG AND NOT QSBUILD_EMSCRIPTEN AND NOT CMAKE_BUILD_TYPE MATCHES Debug)
	target_compile_options (qs PRIVATE "-flto")
	target_link_options (qs PRIVATE "-flto")
endif()

if (UNIX AND NOT APPLE)
	find_library (RT_LIBRARY rt)
	mark_as_advanced (RT_LIBRARY)
	if (RT_LIBRARY)
		target_link_libraries (qs PRIVATE "${RT_LIBRARY}")
	endif()

	find_library (MATH_LIBRARY m)
	mark_as_advanced (MATH_LIBRARY m)
	if (MATH_LIBRARY)
		target_link_libraries (qs PRIVATE "${MATH_LIBRARY}")
	endif()

	if (CMAKE_DL_LIBS)
		target_link_libraries (qs PRIVATE "${CMAKE_DL_LIBS}")
	endif()
endif()

if (UNIX)
	target_compile_options (qs PRIVATE "-fvisibility=hidden")
endif()
