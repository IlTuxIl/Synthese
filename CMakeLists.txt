cmake_minimum_required(VERSION 3.3)
project(Synthese)

INCLUDE(FindPkgConfig)

file( GLOB gKit_SOURCES gKit/*.cpp )

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES ${gKit_SOURCES} src/tp1.cpp src/main.cpp src/Shader.cpp src/Shader.h src/LoadMesh.cpp src/LoadMesh.h)
#add_executable(Synthese ${SOURCE_FILES})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

include_directories(gKit ${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} gKit)
add_executable(Synthese ${SOURCE_FILES})
target_link_libraries(Synthese ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${GLEW_LIBRARY})
