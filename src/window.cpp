#include <iostream>
#include <fstream>
#include <sstream>
#include "../include/window.h"

Window_Manager window_manager;

void check_shader_compilation(GLuint shader) {
    int compile_result;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);
    if (!compile_result) {
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cout << log << std::endl;
    }
}

void check_shader_linkage(GLuint shader) {
    int link_result;
    char log[512];
    glGetShaderiv(shader, GL_LINK_STATUS, &link_result);
    if (!link_result) {
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cout << log << std::endl;
    }
}

const char* read_shader_file(const char* filename) {
    // Open file
    std::fstream shader_file(filename);
    std::string src_str;
    const char* shader_src = nullptr; 
    if (shader_file.is_open()) {
        std::stringstream src_str_stream;
        std::string line;
        while (std::getline(shader_file, line)) {
            src_str_stream << line << "\n";
        }
        src_str = src_str_stream.str();
        // Copy the string over to a char array
        shader_src = new char[src_str.length()];
        // + 1 here to include null character
        std::memcpy((void*)shader_src, src_str.c_str(), src_str.length() + 1);
    } else {
        std::cout << "ERROR: Cannot open shader source file!" << std::endl;
    }
    // close file
    shader_file.close();
    return shader_src;
}

void load_shader_source(const char* vert_filename, 
                                const char* frag_filename,
                                GLuint vertex_shader,
                                GLuint fragment_shader) {
    const char* vert_shader_src = read_shader_file(vert_filename);
    const char* frag_shader_src = read_shader_file(frag_filename);
    glShaderSource(vertex_shader, 1, &vert_shader_src, nullptr);
    glShaderSource(fragment_shader, 1, &frag_shader_src, nullptr);
}

void generate_shader_program(GLuint vertex_shader, GLuint fragment_shader, GLuint shader_program) {
    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    check_shader_compilation(vertex_shader);
    check_shader_compilation(fragment_shader);
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    check_shader_linkage(shader_program);
}

void Window_Manager::create_window() {

}

void Window_Manager::init_window(Window& window) {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    window.shader = glCreateProgram();
    load_shader_source("shaders/shader.vert", "shaders/shader.frag", vertex_shader, fragment_shader);
    generate_shader_program(vertex_shader, fragment_shader, window.shader);
}
