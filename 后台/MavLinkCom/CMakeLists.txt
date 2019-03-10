cmake_minimum_required (VERSION 2.8)

project (MavLinkCom)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -g -O0")

include_directories (
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/mavlink
	${PROJECT_SOURCE_DIR}/mavlink/common
	${PROJECT_SOURCE_DIR}/common_utils
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/src/impl
	${PROJECT_SOURCE_DIR}/src/serial_com
	${PROJECT_SOURCE_DIR}/MavLinkTest
)

add_subdirectory(common_utils)
add_subdirectory(src)
add_subdirectory(MavLinkTest)

