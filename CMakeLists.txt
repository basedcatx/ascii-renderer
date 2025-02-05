cmake_minimum_required(VERSION 3.20)

project(ASCII-VIDEO-RENDERER)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
find_package(Curses REQUIRED)
find_package(SDL2 REQUIRED)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

add_executable(
	ascii-renderer

	main.c
	utils/ffmpeg-utils.c
)

target_include_directories(
	ascii-renderer

	PRIVATE
	${FFMPEG_INCLUDE_DIRS}
	${CURSES_INCLUDE_DIRS}
	${SDL2_INCLUDE_DIRS}
)

target_link_libraries(
	ascii-renderer

	PRIVATE
	${FFMPEG_LIBRARIES}
	${CURSES_LIBRARIES}
	${SDL2_LIBRARIES}
)


target_compile_options(
	ascii-renderer

	PRIVATE
	${FFMPEG_CFLAGS_OTHERS}
	${CURSES_CFLAGS_OTHERS}
	${SDL2_CFLAGS_OTHERS}
)