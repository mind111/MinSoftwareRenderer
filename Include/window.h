#pragma once
#include "../lib/gl/glew/include/glew.h"
#include "../lib/gl/glfw/include/glfw3.h"

struct Window {
    GLFWwindow* m_window;
    int width, height;
    GLuint bitmap_texture;
    GLuint shader;
};

class Window_Manager {
public:
    Window_Manager() {}
    void create_window(Window& window, int width, int height);    
    void init_window(Window& window);
    void blit_buffer(unsigned char* buffer, int buffer_width, int buffer_height, int num_channels, Window& window); 
};

extern Window_Manager window_manager;