# CMakeList.txt : 최상위 CMake 프로젝트 파일, 전역 구성을 수행하고
# 여기에 하위 프로젝트를 포함합니다.
#
cmake_minimum_required (VERSION 3.8)

# 지원되는 경우 MSVC 컴파일러에 대해 핫 다시 로드 사용하도록 설정합니다.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("QsLib" LANGUAGES C VERSION 3.1.1)

# 설정
option (BUILD_SHARED_LIBS "공유 라이브러리 만들기" ON)

include (CMakeDependentOption)
cmake_dependent_option (QSBUILD_WINDOWS "윈도우용으로 만들기" ON "WIN32;NOT EMSCRIPTEN" OFF)
cmake_dependent_option (QSBUILD_WAYLAND "웨이랜드용으로 만들기" ON "UNIX;NOT APPLE;NOT EMSCRIPTEN" OFF)
cmake_dependent_option (QSBUILD_EMSCRIPTEN "EMSCRIPTEN용으로 만들기" ON "EMSCRIPTEN" OFF)
cmake_dependent_option (QSBUILD_SDL2 "SDL2용으로 만들기" OFF "WIN32;UNIX;APPLE" OFF)

if (BUILD_SHARED_LIBS AND NOT QSBUILD_EMSCRIPTEN)
	set (QSBUILD_LIBRARY_TYPE SHARED)
else()
	set (BUILD_SHARED_LIBS OFF)
	set (QSBUILD_LIBRARY_TYPE STATIC)
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang|IntelLLVM")
	set (CLANG TRUE)
endif()

if (QSBUILD_SDL2)
	message (STATUS "SDL2용으로 만들어요")
	set (QSBUILD_WINDOWS OFF)
	set (QSBUILD_WAYLAND OFF)
endif()
if (QSBUILD_EMSCRIPTEN)
	message (STATUS "EMSCRIPTEN용으로 만들어요")
	set (BUILD_SHARED_LIBS OFF)
	set (CMAKE_EXECUTABLE_SUFFIX ".html")
endif()
if (QSBUILD_WINDOWS)
	message (STATUS "윈도우용으로 만들어요")
endif()
if (QSBUILD_WAYLAND)
	message (STATUS "웨이랜드용으로 만들어요")
endif()
if (CLANG)
	message (STATUS "CLANG으로 컴파일해요")
endif()
if (MSVC)
	message (STATUS "MSVC로 컴파일해요")
endif()
if (CMAKE_BUILD_TYPE MATCHES Debug)
	message (STATUS "디버그 모드")
endif()
if (CMAKE_BUILD_TYPE MATCHES Release)
	message (STATUS "릴리즈 모드")
endif()

# 매크로
macro(copy_file target filename destpath)
	if (NOT IS_ABSOLUTE ${filename})
		set (filename ${CMAKE_CURRENT_SOURCE_DIR}/${filename})
	endif()
	add_custom_command (TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different 
			"${filename}" "${destpath}")
endmacro()

# 모든 출력을 같은 위치로
set (LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set (EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

# 공통 설정
include_directories("${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/inc" "${CMAKE_BINARY_DIR}/src")
add_definitions (-DUNICODE -D_UNICODE)
if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions (-D_DEBUG)
endif()

# 하위 프로젝트를 포함합니다.
add_subdirectory ("src")
add_subdirectory ("zconsole")

# 참고: 컴파일러 정의
# echo | clang -dM -E -
