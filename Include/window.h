#pragma once
#include "../lib/gl/glew/include/glew.h"
#include "../lib/gl/glfw/include/glfw3.h"

struct Window {
    GLFWwindow* m_window;
    GLuint bitmap_texture;
    GLuint shader;
};

class Window_Manager {
public:
    Window_Manager() {}
    void create_window();    
    void init_window(Window& window);
};

extern Window_Manager window_manager;