set(FIND_GLFW_PATHS ${CMAKE_SOURCE_DIR}/lib/gl/glfw/lib)
find_path(GLFW_INCLUDE_DIR glfw3.h ${CMAKE_SOURCE_DIR}/lib/gl/glfw/include)
find_library(
    GLFW_LIBRARY 
    NAMES glfw3.lib 
    PATHS ${FIND_GLFW_PATHS})