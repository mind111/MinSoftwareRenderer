set(FIND_GLEW_PATHS ${CMAKE_SOURCE_DIR}/lib/gl/glew/lib)
find_path(GLEW_INCLUDE_DIR glew.h ${CMAKE_SOURCE_DIR}/lib/gl/glew/include)
find_library(GLEW_LIBRARY NAMES glew32s.lib PATHS ${FIND_GLEW_PATHS})