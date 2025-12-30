cmake_minimum_required(VERSION 3.14)
project(MusicPlayer VERSION 1.0 LANGUAGES CXX)

# C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 音频后端选项
option(USE_SFML "Use SFML for audio playback" OFF)
option(USE_WINDOWS "Use Windows MCI for audio playback" ON)

# 头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/main.cpp
)

# 创建可执行文件
add_executable(musicplayer ${SOURCES})

# 根据选择的后端配置
if(USE_SFML)
    find_package(SFML 2.5 COMPONENTS audio REQUIRED)
    target_compile_definitions(musicplayer PRIVATE USE_SFML)
    target_link_libraries(musicplayer sfml-audio)
    message(STATUS "Using SFML audio backend")
elseif(USE_WINDOWS AND WIN32)
    target_link_libraries(musicplayer winmm)
    message(STATUS "Using Windows MCI audio backend")
endif()

# 安装规则
install(TARGETS musicplayer DESTINATION bin)
