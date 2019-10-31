#include "Shader.h"
#include "scene.h"

class Rasterizer {

};

class Renderer {
public:
    std::vector<Shader_Base*> shader_list; // shader pool that include all available style of shaders

    //float* xform_vertex_buffer; save this for later
    uint16_t active_shader_id;
    Mat4x4<float> viewport;

    void init(int w, int h);
    void draw_scene(Scene& scene);

private:
    float* z_buffer;
    void draw_mesh(Mesh& mesh);
    void draw_instance(Scene& scene, Mesh_Instance& mesh_instance);
    void fill_triangle(Vec2<float>& v0, Vec2<float>& v1, Vec2<float>& v2);
};

void Renderer::draw_instance(Scene& scene, Mesh_Instance& mesh_instance) {    
    Shader_Base* active_shader = shader_list[active_shader_id];
    Mesh mesh = scene.mesh_list[mesh_instance.mesh_id];

    for (int f_idx = 0; f_idx < mesh.num_faces; f_idx++) {
        Vec3<float> v0 = mesh_manager.get_vertex(mesh, f_idx * 3);
        Vec3<float> v1 = mesh_manager.get_vertex(mesh, f_idx * 3 + 1);
        Vec3<float> v2 = mesh_manager.get_vertex(mesh, f_idx * 3 + 2);
        
        Vec4<float> v0_clip = active_shader->vertex_shader(v0);
        Vec4<float> v1_clip = active_shader->vertex_shader(v1);
        Vec4<float> v2_clip = active_shader->vertex_shader(v2);

        // perspective divide
        v0_clip = v0_clip / v0_clip.w;
        v1_clip = v1_clip / v1_clip.w;
        v2_clip = v2_clip / v2_clip.w;

        // viewport transform
        Vec2<float> v0_screen(v0_clip.x, v0_clip.y);
        Vec2<float> v1_screen(v1_clip.x, v1_clip.y);
        Vec2<float> v2_screen(v2_clip.x, v2_clip.y);

        // backface cull

        // rasterization
        fill_triangle(v0_screen, v1_screen, v2_screen);
    }
}
