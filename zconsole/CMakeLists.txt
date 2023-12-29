# CMakeList.txt: zconsole에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#

# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_executable (zconsole "zz.c")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET zconsole PROPERTY C_STANDARD 17)
endif()

# 추가
if (MSVC)
	target_link_libraries (zconsole qs)
else()
	target_link_libraries (zconsole PRIVATE qs PUBLIC m)
endif()
