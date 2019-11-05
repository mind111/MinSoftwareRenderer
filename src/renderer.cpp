#include "../include/renderer.h"
#include "Math.h"

Renderer::Renderer() {
    mesh_attrib_flag = 0;
    active_shader_id = 0;
    buffer_width = 0;
    buffer_height = 0;
}

void Renderer::init(int w, int h) {
    z_buffer = new float[w * h];
    // init z_buffer depth value to a huge number
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            z_buffer[w * j + i] = 100.f;
        }
    }
    // setup viewport matrix here
    viewport = Mat4x4<float>::viewport(w, h);
    // setup available shader here
}

void Renderer::alloc_backbuffer(Window& window) {
    buffer_width = window.width;
    buffer_height = window.height;
    backbuffer = new unsigned char[4 * buffer_width * buffer_height];
}

void Renderer::draw_pixel(int x, int y, Vec4<int>& color) {
    backbuffer[(y * buffer_width + x) * 4] = (unsigned char)color.x;     // r
    backbuffer[(y * buffer_width + x) * 4 + 1] = (unsigned char)color.y; // g
    backbuffer[(y * buffer_width + x) * 4 + 2] = (unsigned char)color.z; // b
    backbuffer[(y * buffer_width + x) * 4 + 3] = (unsigned char)color.w; // a
}

void Renderer::draw_instance(Scene& scene, Mesh_Instance& mesh_instance) {    
    Shader_Base* active_shader = shader_list[active_shader_id];
    Mesh mesh = scene.mesh_list[mesh_instance.mesh_id];
    if (mesh.texture_uv_buffer) {
        mesh_attrib_flag = mesh_attrib_flag | 1;
    }
    if (mesh.normal_buffer) {
        mesh_attrib_flag = mesh_attrib_flag | (1 << 1);
    }

    // TODO: @ Clean up
    // render face by face
    for (int f_idx = 0; f_idx < mesh.num_faces; f_idx++) {
        for (int v = 0; v < 3; v++) {
            // vertex transform
            triangle_clip[v] = active_shader->vertex_shader(mesh_manager.get_vertex(mesh, f_idx * 3 + v));
            // perspective division
            triangle_clip[v] = triangle_clip[v] / triangle_clip[v].w;
            // viewport transform
            Vec4<float> v_screen = viewport * triangle_clip[v];
            triangle_screen[v].x = v_screen.x;
            triangle_screen[v].y = v_screen.y;
            // has texture uv attrib 
            if (mesh_attrib_flag & 0x0001) {
                triangle_uv[v] = mesh_manager.get_vt(mesh, f_idx * 3 + v); 
            } 
            // has normal attrib 
            if (mesh_attrib_flag & 0x0010) {
                triangle_normal[v] = mesh_manager.get_vn(mesh, f_idx * 3 + v);
            } 
        }

        // TODO: @ frustum culling
        // if projected triangle is partially out of screen, discard it for now
        
        // backface cull

        // -------------

        // rasterization
        fill_triangle(active_shader);
    }
}

bool Renderer::depth_test(int fragment_x, int fragment_y, Vec3<float> _bary_coord) {
    return false;
}

// TODO: Improve normal mapping
void Renderer::fill_triangle(Shader_Base* active_shader_ptr) {
    // TODO: @ Clean up using a determinant()
    Vec2<float> e1 = triangle_screen[1] - triangle_screen[0];
    Vec2<float> e2 = triangle_screen[2] - triangle_screen[0];
    float denom = e1.x * e2.y - e2.x * e1.y;
    // discard the triangle if its degenerated into a line
    if (denom == 0.f) {
        return;
    }

    float bbox[4] = {
        triangle_screen[0].x,
        triangle_screen[0].x,
        triangle_screen[0].y,
        triangle_screen[0].y,
    }; 
    Math::bound_triangle(triangle_screen, bbox);

    for (int x = bbox[0]; x < bbox[1]; x++) {
        for (int y = bbox[2]; y < bbox[3]; y++) {
            // compute barycentric coord
            Vec3<float> bary_coord = Math::barycentric(triangle_screen, x, y, denom);
            // overlapping test
            if (bary_coord.x < 0.f || bary_coord.y < 0.f || bary_coord.z < 0.f) {
                continue;
            }
            // depth test
            if (!depth_test(x, y, bary_coord)) {
                continue;
            }
            // TODO: may not even need to bother checking
            // interpolate given vertex attribute
            if (mesh_attrib_flag & 0x0001) {
                active_shader_ptr->fragment_texture_coord = Math::bary_interpolate(triangle_uv, bary_coord);
            }
            if (mesh_attrib_flag & 0x0010) {
                active_shader_ptr->fragment_normal = Math::bary_interpolate(triangle_normal, bary_coord);
            }
            // compute fragment color
            Vec4<float> fragment_color = active_shader_ptr->fragment_shader(x, y);
            // write to backbuffer
            // TODO: @ How to interface with SDL

            // -------------------
        }
    }
}