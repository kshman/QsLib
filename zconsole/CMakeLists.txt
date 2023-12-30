# CMakeList.txt: zconsole에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#

# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_executable (zconsole "zz.c")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET zconsole PROPERTY C_STANDARD 17)
endif()

# 추가
target_link_libraries (zconsole PRIVATE qs)

if (QSLIB_BUILD_WINDOWS)
	target_compile_options (zconsole PRIVATE "/arch:AVX2" "/fp:fast")
endif()

if (UNIX AND NOT APPLE)
	find_library (RT_LIBRARY rt)
	mark_as_advanced (RT_LIBRARY)
	if (RT_LIBRARY)
		target_link_libraries (zconsole PRIVATE "${RT_LIBRARY}")
	endif()

	find_library (MATH_LIBRARY m)
	mark_as_advanced (MATH_LIBRARY m)
	if (MATH_LIBRARY)
		target_link_libraries (zconsole PRIVATE "${MATH_LIBRARY}")
	endif()
endif()
