cmake_minimum_required(VERSION 3.16)

# Sets project name
project(projects_realtime LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp

    src/mainwindow.h
    src/realtime.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp

    src/shapes/Cone.cpp
    src/shapes/Cone.h
    src/shapes/Cube.cpp
    src/shapes/Cube.h
    src/shapes/Cylinder.cpp
    src/shapes/Cylinder.h
    src/shapes/Sphere.cpp
    src/shapes/Sphere.h

    src/camera/camera.h
    src/camera/camera.cpp

    src/cameramove.cpp
    src/utils/realtimeutils.cpp

    src/floor.cpp
    src/floor.h
    src/fountain.cpp
    src/fountain.h
    src/texture.cpp
    src/texture.h
    src/shader.cpp
    src/shader.h
    src/stb_image.c
    src/stb_image.h
    src/mflame.cpp
    src/mflame.h
)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c)
include_directories(${PROJECT_NAME} PRIVATE glew/include)



# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
        resources/shaders/phong.frag
        resources/shaders/phong.vert
        resources/shaders/postprocess.frag
        resources/shaders/postprocess.vert
        resources/shaders/floor.frag
        resources/shaders/floor.vert
        resources/shaders/Update.vs
        resources/shaders/Update.fs
        resources/shaders/Update.gs
        resources/shaders/Render.vs
        resources/shaders/Render.fs
        resources/water.bmp

        resources/shaders/FlameUpdate.vs
        resources/shaders/FlameUpdate.fs
        resources/shaders/FlameUpdate.gs
        resources/shaders/FlameRender.vs
        resources/shaders/FlameRender.fs
        resources/flame.bmp
        resources/particle.bmp


)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
