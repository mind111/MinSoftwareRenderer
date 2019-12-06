cmake_minimum_required(VERSION 3.0)
project(minSoftwareRenderer)
#### source files ####
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE LIBS "${PROJECT_SOURCE_DIR}/lib/stb_image/*.cpp" 
                       "${PROJECT_SOURCE_DIR}/lib/stb_image/*.h"
                       "${PROJECT_SOURCE_DIR}/lib/json/*.h")
#### coying data folders ####
file(GLOB ASSETS "${PROJECT_SOURCE_DIR}/assets/*")
file(GLOB SHADERS "${PROJECT_SOURCE_DIR}/shaders/*")
file(GLOB SCENES "${PROJECT_SOURCE_DIR}/scenes")
#### preprocessor ####
add_definitions(-DGLEW_STATIC)
add_executable(minSoftwareRenderer ${HEADERS} ${SOURCES} ${LIBS})
#### external dependencies ####
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenMP REQUIRED)
message(${GLFW_LIBRARY})
include_directories(${GLFW_INCLUDE_DIR})
link_directories(minSoftwareRenderer PUBLIC ${GLFW_LIBRARY})
find_package(GLEW 2.0 REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
link_directories(minSoftwareRenderer PUBLIC ${GLEW_LIBRARY})
message(${GLEW_LIBRARY})
target_compile_options(minSoftwareRenderer PUBLIC /W1)
target_link_options(minSoftwareRenderer PUBLIC /NODEFAULTLIB:library)
#### linking to openmp ####
if (OpenMP_CXX_FOUND)
    target_link_libraries(minSoftwareRenderer OpenMP::OpenMP_CXX ${GLFW_LIBRARY} ${GLEW_LIBRARY} ${OPENGL_LIBRARIES})
else()
    #### linking to all required libs ####
    target_link_libraries(minSoftwareRenderer ${GLFW_LIBRARY} ${GLEW_LIBRARY} ${OPENGL_LIBRARIES})
endif()
#### copy all the data into bin folder to allow relative pathing ####
file(COPY ${ASSETS} DESTINATION assets)
file(COPY ${SHADERS} DESTINATION shaders)
file(COPY ${SCENES} DESTINATION ${CMAKE_BINARY_DIR})