#include "omp.h"
#include "renderer.h"

Renderer::Renderer() {
    mesh_attrib_flag = 0;
    activeShaderPtr_ = nullptr;
    buffer_width = 0;
    buffer_height = 0;
}

// depth bit is clear to 1.5 instead of 1 since later cubemap's depth is set to
//  1. This way it avoid z-fighting when render cubemap
void Renderer::init() {
    z_buffer = new float[buffer_width * buffer_height];
    // init z_buffer depth value to a huge number
    for (int j = 0; j < buffer_height; j++) {
        for (int i = 0; i < buffer_width; i++) {
            z_buffer[buffer_width * j + i] = 1.5f;
        }
    }
    // setup viewport matrix here
    viewport = Mat4x4<float>::viewport(buffer_width, buffer_height);
    // setup available shader here
}

void Renderer::alloc_backbuffer(Window& window) {
    buffer_width = window.width;
    buffer_height = window.height;
    backbuffer = new unsigned char[4 * buffer_width * buffer_height];
}

// @: Flipped the y coordinates to match OpenGL's screen space coordinates
//    since I later pass this buffer as a texture data for OpenGL to render
void Renderer::draw_pixel(int x, int y, Vec4<int>& color) {
    backbuffer[(y * buffer_width + x) * 4] = (unsigned char)color.x;     // r
    backbuffer[(y * buffer_width + x) * 4 + 1] = (unsigned char)color.y; // g
    backbuffer[(y * buffer_width + x) * 4 + 2] = (unsigned char)color.z; // b
    backbuffer[(y * buffer_width + x) * 4 + 3] = (unsigned char)color.w; // a
}

void Renderer::clearBuffer() {
    for (int pixel = 0; pixel < buffer_width * buffer_height; pixel++) {
        backbuffer[pixel * 4] = 0;
        backbuffer[pixel * 4 + 1] = 0;
        backbuffer[pixel * 4 + 2] = 0;
        backbuffer[pixel * 4 + 3] = 255;
    }
}

void Renderer::clearDepth() {
    for (int pixel = 0; pixel < buffer_width * buffer_height; pixel++) {
        z_buffer[pixel] = 1.5f;
    }
}

void Renderer::drawSkybox(Scene& scene) {
    Mesh skyboxMesh = scene.mesh_list[scene.skyboxMeshID];
}

void Renderer::drawScene(Scene& scene) {
    // TODO: view should be updated per frame later when I impl camera control
    Mat4x4<float> view = scene_manager.get_camera_view(scene.main_camera);
    Mat4x4<float> projection = Mat4x4<float>::Perspective((float)buffer_width / (float)buffer_height, scene.main_camera.z_near, scene.main_camera.z_far, scene.main_camera.fov);
    activeShaderPtr_->set_view_matrix(view);
    activeShaderPtr_->set_projection_matrix(projection);
    skyboxShader_->set_view_matrix(view);
    skyboxShader_->set_projection_matrix(projection);
    for (auto& instance : scene.instance_list) {
        for (auto directionalLight : scene.directionalLightList) {
            Mesh& mesh = scene.mesh_list[instance.mesh_id];
            // bind the texture to active shader
            if (mesh.textureID != -1) {
                activeShaderPtr_->bindTexture(&scene.texture_list[mesh.textureID]);
            }
            if (mesh.normalMapID != -1) {
                activeShaderPtr_->normalMap_ = &scene.texture_list[mesh.normalMapID];  
            }
            Mat4x4<float> model = Math::constructTransformMatrix(scene.xform_list[instance.instance_id]);
            activeShaderPtr_->set_model_matrix(model);
            draw_instance(&directionalLight, mesh);
            // clear shader's per fragment attrib buffer
            activeShaderPtr_->clearFragmentAttribs();
            // unbind texture and normal map
            activeShaderPtr_->unbindTexture();
            activeShaderPtr_->normalMap_ = nullptr;
        }
    }
    if (scene.skyboxMeshID != -1) {
        Shader_Base* oldShaderPtr = activeShaderPtr_;
        // render skybox
        activeShaderPtr_ = skyboxShader_;
        for (auto& directionalLight : scene.directionalLightList) {
            draw_instance(&directionalLight, scene.mesh_list[scene.skyboxMeshID]);
        }
        activeShaderPtr_ = oldShaderPtr;
    }
}

// TODO: @ Frustum culling
// TODO: @ Backface culling
// TODO: @ PBR
// TODO: @ Should deal with instance xform in here
void Renderer::draw_instance(Light* light, Mesh& mesh) {    
    mesh_attrib_flag = 0; // reset the flag
    if (mesh.texture_uv_buffer) {
        mesh_attrib_flag = mesh_attrib_flag | 1;
    }
    if (mesh.normal_buffer) {
        mesh_attrib_flag = mesh_attrib_flag | (1 << 1);
    }
    if (mesh.tangentBuffer) {
        mesh_attrib_flag = mesh_attrib_flag | (1 << 2);
    }

    activeShaderPtr_->vertexAttribFlag = mesh_attrib_flag;

    // TODO: @ Clean up
    // TODO: @ Debug rendering
    // TODO: @ OpenMP
    // render face by face
    for (int f_idx = 0; f_idx < mesh.num_faces; f_idx++) {
        for (int v = 0; v < 3; v++) {
            // vertex transform
            Vec3<float> vertex = mesh_manager.get_vertex(mesh, f_idx * 3 + v);
            triangle_clip[v] = activeShaderPtr_->vertex_shader(vertex);
            if (mesh.tangentBuffer) {
                Vec3<float> vertexTangent = mesh_manager.getTangent(mesh, f_idx * 3 + v);
                //drawTangents(mesh_manager.get_vertex(mesh, f_idx * 3 + v), vertexTangent);
            }
            // viewport transform
            Vec4<float> v_screen = viewport * (triangle_clip[v] / triangle_clip[v].w);
            triangle_screen[v].x = v_screen.x;
            triangle_screen[v].y = v_screen.y;
            // has texture uv attrib 
            if (mesh_attrib_flag & 0x0001) {
                triangle_uv[v] = mesh_manager.get_vt(mesh, f_idx * 3 + v); 
            } 
            // has normal attrib 
            if (mesh_attrib_flag & 0x0002) {
                // TODO: do correct transformation for normals and tangents that will work for any
                // transformation involving non-uniform scaling. For now, since I am not doing any scaling
                // so using the same modelView transform should be suffice for now
                // need to transform the normal here
                normalIn[v] = mesh_manager.get_vn(mesh, f_idx * 3 + v);
                normalOut[v] = activeShaderPtr_->transformNormal(normalIn[v]);
            } 
            // has vertex tangent
            if (mesh_attrib_flag & 0x0004) {
                tangentIn[v] = mesh_manager.getTangent(mesh, f_idx * 3 + v);
                tangentOut[v] = activeShaderPtr_->transformNormal(tangentIn[v]);
            }
        }

        // TODO: @ frustum culling
        // if projected triangle is partially out of screen, discard it for now
        
        // backface cull

        // -------------

        // draw out mesh wireframe for debugging purposes

        // rasterization
        fill_triangle(activeShaderPtr_, light);
    }
}

bool Renderer::depth_test(int fragmentX, int fragmentY, Vec3<float> _bary_coord) {
    int index = fragmentY * buffer_width + fragmentX;
    // Maybe this is wrong here, I was using the z after perspective division
    float fragmentZ = 1 / (_bary_coord.x / triangle_clip[0].z + _bary_coord.y / triangle_clip[1].z + _bary_coord.z / triangle_clip[2].z);
    if (fragmentZ < z_buffer[index]) {
        z_buffer[index] = fragmentZ;
        return true;
    }
    return false;
}

// TODO: Improve normal mapping
// TODO: Create multiple fragmentShader instance
// TODO: Maybe something like fragmentShaderPools[bufferWidth][bufferHeight]
void Renderer::fill_triangle(Shader_Base* active_shader_ptr, Light* light) {
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
    // TODO: clamp
    int xMin = std::floor(bbox[0]), xMax = std::ceil(bbox[1]);
    int yMin = std::floor(bbox[2]), yMax = std::ceil(bbox[3]);
    // TODO: @ OpenMP
    //omp_set_num_threads(4);
    //#pragma omp parallel for 
    for (int x = xMin; x <= xMax; x++) {
        for (int y = yMin; y <= yMax; y++) {
            // compute barycentric coord
            Vec3<float> bary_coord = Math::barycentric(triangle_screen, x, y, denom); // overlapping test
            if (bary_coord.x < 0.f || bary_coord.y < 0.f || bary_coord.z < 0.f) {
                continue;
            }
            // depth test
            if (!depth_test(x, y, bary_coord)) {
                continue;
            }
            // get fragment depth

            // TODO: may not even need to bother checking
            // TODO: if normal is not provided, pass in interpolated normal
            // interpolate given vertex attribute
            uint32_t attribIdx = y * buffer_width + x; 
            // TODO: @ pass in light to fragment shader
            if (mesh_attrib_flag & 0x0001) {
                active_shader_ptr->fragmentAttribBuffer[attribIdx].textureCoord = Math::bary_interpolate(triangle_uv, bary_coord);
            }
            if (mesh_attrib_flag & 0x0002) {
                active_shader_ptr->fragmentAttribBuffer[attribIdx].normal = Math::bary_interpolate(normalOut, bary_coord);
            }
            if (mesh_attrib_flag & 0x0004) {
                active_shader_ptr->fragmentAttribBuffer[attribIdx].tangent = Math::bary_interpolate(tangentOut, bary_coord);
            }
            // TODO: Bulletproof this setup for lighting computation
            // point light
            if (light->getPosition()) {
                // compute per fragment light direction
            } else {
                if (active_shader_ptr->lightingParamBuffer) {
                    active_shader_ptr->lightingParamBuffer[attribIdx].color = light->color;
                    active_shader_ptr->lightingParamBuffer[attribIdx].intensity = light->intensity;
                    active_shader_ptr->lightingParamBuffer[attribIdx].direction = *(light->getDirection());
                }
                // view
            }
            // compute fragment color
            Vec4<int> fragmentColor = active_shader_ptr->fragment_shader(x, y);
            // write to backbuffer
            draw_pixel(x, y, fragmentColor);
            // -------------------
        }
    }
}

void Renderer::drawLine(Vec2<int> start, Vec2<int> end) {
    int d = 1;
    //**** Divide by 0
    if (start.x == end.x)
    {
        if (start.y > end.y) start.Swap(end);
        for (int i = start.y; i <= end.y; i++)
        {
            Vec4<int> color(50, 0, 250, 255);
            draw_pixel(start.x, i, color);
        }

        return;
    }
    // Always start from left marching toward right
    // Swap start and end
    if (start.x > end.x) start.Swap(end);
    // Derive the line function
    float slope = (float)(start.y - end.y) / (float)(start.x - end.x);
    float intercept = (float)end.y - slope * end.x;
    Vec2<int> next(start);
    Vec2<int> stepA(1, 0);
    Vec2<int> stepB(1, 1);

    if (slope < 0)
    {
        d *= -1;
        stepA.Swap(stepB);
    }
    if (slope > 1 || slope < -1)
    {
        stepA.Transpose();
        stepB.Transpose();
    }

    stepA.y *= d;
    stepB.y *= d;

    while (next.x < end.x || next.y != end.y)
    {
        //**** slope < 1
        if (slope >= -1 && slope <= 1)
        {
            // Eval F(x+1,y+0.5)
            if (next.y + d * 0.5f - slope * (next.x + 1.0f) - intercept >= 0)
                next += stepA;
            else
                next += stepB;
        }
        /// \Bug: When slope is greater than 0, can invert x, y
        else
        {
            // Eval F(x+0.5, y+1)
            if (next.y + d * 1.0f - slope * (next.x + .5f) - intercept >= 0)
                next += stepB;
            else
                next += stepA;
        }
        //**** Draw pixel to the buffer
        Vec4<int> color(50, 0, 250, 255);
        draw_pixel(next.x, next.y, color);
    }
}