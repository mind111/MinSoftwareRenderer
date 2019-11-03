#include "Shader.h"
#include "scene.h"
#include <bitset>

class Rasterizer {

};

struct Attrib_Ptr {
    void* data;
    int num_components;
};

class Renderer {
public:
    std::vector<Shader_Base*> shader_list; // shader pool that include all available style of shaders

    //float* xform_vertex_buffer; save this for later
    // This maybe refactored to be part of rasterizer
    Vec4<float> triangle_clip[3];
    Vec2<float> triangle_screen[3];
    Vec3<float> triangle_uv[3]; // if uv only has two components then fill z with 0
    Vec3<float> triangle_normal[3];

    uint8_t mesh_attrib_flag;

    uint16_t active_shader_id;
    Mat4x4<float> viewport;
    float* backbuffer;

    Renderer();
    void init(int w, int h);
    void bind_mesh_buffers(Mesh& mesh);
    void draw_scene(Scene& scene);

private:
    float* z_buffer;
    void draw_mesh(Mesh& mesh);
    void draw_instance(Scene& scene, Mesh_Instance& mesh_instance);
    bool depth_test(int fragment_x, int fragment_y, Vec3<float> _bary_coord);
    void fill_triangle(Shader_Base* active_shader_ptr);
};

