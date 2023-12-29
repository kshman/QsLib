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
include_directories("./inc" "./src")

add_definitions (-DUNICODE -D_UNICODE)

if (CMAKE_C_COMPILER_ID MATCHES "Clang|IntelLLVM")
	set (CLANG TRUE)
	if (MSVC)
		set (MSVC_CLANG TRUE)
	endif()
endif()

set (LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set (EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

# 하위 프로젝트를 포함합니다.
add_subdirectory ("src")
add_subdirectory ("zconsole")

add_dependencies (zconsole qs)
