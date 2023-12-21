CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)

# 프로젝트
PROJECT(QsLib LANGUAGES C)
SET(PROJECT_VERSION_MAJOR 3)
SET(PROJECT_VERSION_MINOR 1)

INCLUDE(CTest)
enable_testing()

# 빌드 타입
SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_VERBOSE_MAKEFILE true)

# 도구 설정
SET(CMAKE_C_STANDARD 17)

# 플랫폼과 컴파일러 옵션
SET(PLATFORM ${CMAKE_HOST_SYSTEM_PROCESSOR})
MESSAGE(STATUS "플랫폼: ${PLATFORM}")

if (UNIX)
	ADD_COMPILE_OPTIONS(-W -Wall -Wextra -fvisibility=hidden)
endif()
if (${CMAKE_SYSTEM_NAME} MATCHES FreeBSD)
	ADD_LINK_OPTIONS(-L/usr/local/lib)
endif()
if (${CMAKE_C_COMPILER_ID} MATCHES Clang)
	MESSAGE(STATUS "FLTO 사용!")
	ADD_COMPILE_OPTIONS(-flto)
	ADD_LINK_OPTIONS(-flto)
	if (${PLATFORM} MATCHES AMD64 OR ${PLATFORM} MATCHES amd64 OR ${PLATFORM} MATCHES x86_64)
		MESSAGE(STATUS "AVX2 사용!")
		ADD_COMPILE_OPTIONS(-mavx2)
	endif()
endif()

# 소스 위치
SET(QS_SRC_DIR ${CMAKE_SOURCE_DIR}/../../src)
SET(PATH_ZLIB ${QS_SRC_DIR}/qn/zlib)
SET(PATH_QN ${QS_SRC_DIR}/qn)
SET(QS_SRCS
	${PATH_ZLIB}/adler32.c ${PATH_ZLIB}/compress.c ${PATH_ZLIB}/crc32.c ${PATH_ZLIB}/deflate.c ${PATH_ZLIB}/gzclose.c ${PATH_ZLIB}/gzlib.c ${PATH_ZLIB}/gzread.c ${PATH_ZLIB}/gzwrite.c ${PATH_ZLIB}/infback.c ${PATH_ZLIB}/inffast.c ${PATH_ZLIB}/inflate.c ${PATH_ZLIB}/inftrees.c ${PATH_ZLIB}/trees.c ${PATH_ZLIB}/uncompr.c ${PATH_ZLIB}/zutil.c
	${PATH_QN}/PatrickPowell_snprintf.c
	${PATH_QN}/qn.c ${PATH_QN}/qn_debug.c ${PATH_QN}/qn_file.c ${PATH_QN}/qn_gam.c ${PATH_QN}/qn_hash.c ${PATH_QN}/qn_math.c ${PATH_QN}/qn_mem.c ${PATH_QN}/qn_mlu.c ${PATH_QN}/qn_sort.c ${PATH_QN}/qn_str.c ${PATH_QN}/qn_thd.c ${PATH_QN}/qn_time.c)
INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/../../inc
	${QS_SRC_DIR})

# 만들기
ADD_LIBRARY(qs SHARED ${QS_SRCS})

# 인스톨
INSTALL(TARGETS qs DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

SET(CPACK_PROJECT_NAME ${PROJECT_NAME})
SET(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
INCLUDE(CPack)
