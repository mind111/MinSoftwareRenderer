#include <thread>
#include <future>
#include "omp.h"
#include "renderer.h"

float gSampleRadius = 0.f;

Renderer::Renderer() {
    mesh_attrib_flag = 0;
    activeShaderPtr_ = nullptr;
    bufferWidth_ = 0;
    bufferHeight_ = 0;
    normalBuffer = nullptr;
}

// depth bit is clear to 1.5 instead of 1 since later cubemap's depth is set to
//  1. This way it avoid z-fighting when render cubemap
void Renderer::init() {
    zBuffer = new float[bufferWidth_ * bufferHeight_];
    // init z_buffer depth value to a huge number
    for (int j = 0; j < bufferHeight_; j++) {
        for (int i = 0; i < bufferWidth_; i++) {
            zBuffer[bufferWidth_ * j + i] = 1.5f;
        }
    }
    // setup viewport matrix here
    viewport = Mat4x4<float>::viewport(bufferWidth_, bufferHeight_);
    // normal buffer
    normalBuffer = new Vec3<float>[bufferWidth_ * bufferHeight_];
}

void Renderer::alloc_backbuffer(Window& window) {
    bufferWidth_ = window.width;
    bufferHeight_ = window.height;
    backbuffer = new unsigned char[4 * bufferWidth_ * bufferHeight_];
}

bool Renderer::backfaceCulling(Vec3<float>* vPosView) {
    Vec3<float> v = Math::normalize(vPosView[0] * -1.f);
    Vec3<float> e1 = Math::normalize(vPosView[1] - vPosView[0]);    
    Vec3<float> e2 = Math::normalize(vPosView[2] - vPosView[0]);    
    Vec3<float> facetNormal = Math::CrossProduct(e1, e2);
    if (Math::dotProductVec3(facetNormal, v) < 0.f) {
        return true;
    }
    return false;
}

// @: Flipped the y coordinates to match OpenGL's screen space coordinates
//    since I later pass this buffer as a texture data for OpenGL to render
void Renderer::draw_pixel(int x, int y, Vec4<int>& color) {
    backbuffer[(y * bufferWidth_ + x) * 4] = (unsigned char)color.x;     // r
    backbuffer[(y * bufferWidth_ + x) * 4 + 1] = (unsigned char)color.y; // g
    backbuffer[(y * bufferWidth_ + x) * 4 + 2] = (unsigned char)color.z; // b
    backbuffer[(y * bufferWidth_ + x) * 4 + 3] = (unsigned char)color.w; // a
}

void Renderer::clearBuffer(Vec3<int>& clearColor) {
    #pragma omp parallel for
    for (int pixel = 0; pixel < bufferWidth_ * bufferHeight_; pixel++) {
        backbuffer[pixel * 4] = clearColor.x;
        backbuffer[pixel * 4 + 1] = clearColor.y;
        backbuffer[pixel * 4 + 2] = clearColor.z;
        backbuffer[pixel * 4 + 3] = 255;
    }
}

void Renderer::clearDepth() {
    #pragma omp parallel for
    for (int pixel = 0; pixel < bufferWidth_ * bufferHeight_; pixel++) {
        zBuffer[pixel] = 1.5f;
    }
}

void Renderer::clearNormalBuffer() {
    #pragma omp parallel for
    for (int pixel = 0; pixel < bufferWidth_ * bufferHeight_; pixel++) {
        normalBuffer[pixel] = {};
    }
}

void Renderer::drawSkybox(Scene& scene) {
    Mesh skyboxMesh = scene.mesh_list[scene.skyboxMeshID];
}

// Reconstruct view space position from fragment pos and depth
Vec3<float> Renderer::reconstructViewPosFromDepth(int x, int y, float depth) {
    float radius = 10.f;
    float ndcX = x * 2.f / bufferWidth_ - 1.f; 
    float ndcY = y * 2.f / bufferHeight_ - 1.f; 
    Vec4<float> ndcPos(x, y, depth, 1.f);
    Vec4<float> viewHomo = activeShaderPtr_->projection_.Inverse() * ndcPos; 
    viewHomo = viewHomo / viewHomo.w;
    return Vec3<float>(viewHomo.x, viewHomo.y, viewHomo.z);
}

void Renderer::sampleOcclusion(int x, int y, Vec2<float> dir) {
    if (zBuffer[y * bufferWidth_ + x] >= 1.5f) return;
    Vec3<float> v = reconstructViewPosFromDepth(x, y, zBuffer[y * bufferWidth_ + x]);
    float ao = 0.f;
    for (float step = 0; step < gSampleRadius; step++) {
        Vec2<int> p = Vec2<int>((int)(x + dir.x * step), (int)(y + dir.y * step));
        if (p.x >= bufferWidth_ || p.x < 0 || p.y >= bufferHeight_ || p.y < 0) {
            continue;
        }
        float depth = zBuffer[p.y * bufferWidth_ + p.x]; 
        if (depth == 1.5f) {
            continue;
        }
        Vec3<float> pView = reconstructViewPosFromDepth(p.x, p.y, depth);
        Vec3<float> vp = Math::normalize(pView - v);
        Vec3<float> normal = normalBuffer[p.y * bufferWidth_ + x];
        float occlusion = Math::dotProductVec3(normal, vp); 
    }
}

float Renderer::doAmbientOcclusion(int x, int y, Vec2<float>& sampleDir) {
    float radius = 10.f;
    float ao = 1.f;
    Vec3<float> p(2.f * x / bufferHeight_ - 1.f, 2.f * y / bufferHeight_ - 1.f, zBuffer[y * bufferWidth_ + x]);
    for (float i = 0; i < radius; i += 1.f) {
        int targetX = i * sampleDir.x + x, targetY = i * sampleDir.y + y;
        if (targetX >= bufferWidth_ || targetX < 0 || targetY >= bufferHeight_ || targetY < 0) {
            continue;
        }
        float targetDepth = zBuffer[targetY * bufferWidth_ + targetX];  
        if (targetDepth >= 1.5f) {
            continue;
        }
        Vec3<float> v = Math::normalize(Vec3<float>(2.f * targetX / bufferHeight_ - 1.f , 2.f * targetY / bufferHeight_ - 1.f, targetDepth) - p);
        Vec3<float> pPrime = Math::normalize(Vec3<float>(v.x, v.y, 0.f));
        float cos = Math::dotProductVec3(pPrime, v);
        if (cos < ao) {
            ao = cos; 
        }
    }
    return (1 - ao);
}

float Renderer::ambientOcclusion(int x, int y, Vec2<float>& sampleDir) {
    float ao = 1.f, depth = zBuffer[y * bufferWidth_ + x];
    Vec3<float> reconstructViewPosFromDepth(x, y, depth);
    return (1 - ao);
}

// TODO:: Have a bounding box in screen space to bound the whole scene
//        to reduce number of pixels to traversal
void Renderer::SSAO() {
    int numOfSamples = 16;
    float anglePerSample = 2 * PI / (float)numOfSamples;
    #pragma omp parallel for
    for (int y = 0; y < bufferHeight_; y++) {
        for (int x = 0; x < bufferWidth_; x++) {
            int pixelIdx = y * bufferWidth_ + x;
            float depth = zBuffer[y * bufferWidth_ + x];
            if (depth == 1.5f) {
                continue;                
            }
            float ao = 0.f;
            for (int i = 0; i < numOfSamples; i++) {
                Vec2<float> sampleDir(std::cos(i * anglePerSample), std::sin(i * anglePerSample));
                ao += doAmbientOcclusion(x, y, sampleDir);
            }
            ao /= numOfSamples;
            for (int channel = 0; channel < 3; channel++) {
                // pow to increase the contrast of the shadow around edges
                backbuffer[pixelIdx * 4 + channel] *= pow(1 - ao, 5.f);
            }
        }
    }
}

// TODO: Clean up these nasty ifs??
void Renderer::bindMeshTextures(Scene& scene, const Mesh& mesh) {
    // bind the texture to active shader
    if (mesh.diffuseMapTable.size() > 0) {
        for (auto itr = mesh.diffuseMapTable.begin(); itr != mesh.diffuseMapTable.end(); itr++) {
            activeShaderPtr_->bindDiffuseTexture(&scene.textureList[itr->second]);
        }
    }
    if (mesh.specularMapTable.size() > 0) {
        for (auto itr = mesh.specularMapTable.begin(); itr != mesh.specularMapTable.end(); itr++) {
            activeShaderPtr_->bindSpecTexture(&scene.textureList[itr->second]);
        }
    }
    if (mesh.normalMapID != -1) {
        activeShaderPtr_->normalMap_ = &scene.textureList[mesh.normalMapID];  
    }
    if (mesh.roughnessMapID != -1) {
        activeShaderPtr_->roughnessMap_ = &scene.textureList[mesh.roughnessMapID];
    }
    if (mesh.aoMapID != -1) {
        activeShaderPtr_->aoMap_ = &scene.textureList[mesh.aoMapID];
    }
}

void Renderer::drawScene(Scene& scene) {
    // TODO: view should be updated per frame later when I impl camera control
    activeShaderPtr_->cameraPos = scene.main_camera.position; 
    Mat4x4<float> view = scene_manager.get_camera_view(scene.main_camera);
    Mat4x4<float> projection = Mat4x4<float>::Perspective((float)bufferWidth_ / (float)bufferHeight_, scene.main_camera.z_near, scene.main_camera.z_far, scene.main_camera.fov);
    activeShaderPtr_->set_view_matrix(view);
    activeShaderPtr_->set_projection_matrix(projection);
    skyboxShader_->set_view_matrix(view);
    skyboxShader_->set_projection_matrix(projection);

    // Setup lighting
    activeShaderPtr_->dirLights.clear();
    for (auto directionalLight : scene.directionalLightList) {
        activeShaderPtr_->dirLights.push_back(directionalLight);
    }

    for (auto& instance : scene.instance_list) {
        Mesh& mesh = scene.mesh_list[instance.mesh_id];
        Mat4x4<float> model = Math::constructTransformMatrix(scene.xform_list[instance.instance_id]);
        activeShaderPtr_->set_model_matrix(model);
        bindMeshTextures(scene, mesh);
        drawInstance(mesh);
        // clear shader's per fragment attrib buffer
        activeShaderPtr_->clearFragmentAttribs();
        // unbind texture and normal map
        activeShaderPtr_->unbindTexture();
        activeShaderPtr_->normalMap_ = nullptr;
    }
}

// TODO: @ PBR
// TODO: @ Should deal with instance xform in here
void Renderer::drawInstance(Mesh& mesh) {    
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
    // render face by face
    for (int f_idx = 0; f_idx < mesh.num_faces; f_idx++) {
        Vec3<float> vertexPosScreen[3], vertexPosView[3], textureCoords[3], normals[3], tangents[3];
        Vec4<float> vertexPosClip[3];

        for (int v = 0; v < 3; v++) {
            // vertex transform
            Vec3<float> vertex = mesh_manager.get_vertex(mesh, f_idx * 3 + v);
            vertexPosView[v] = activeShaderPtr_->transformToViewSpace(vertex);
            vertexPosClip[v] = activeShaderPtr_->vertexShader(vertex);
            // viewport transform
            Vec4<float> vScreen = viewport * (vertexPosClip[v] / vertexPosClip[v].w);
            vertexPosScreen[v] = Vec3<float>(vScreen.x, vScreen.y, vScreen.z);
            // has texture uv attrib 
            if (mesh_attrib_flag & 0x0001) {
                textureCoords[v] = mesh_manager.get_vt(mesh, f_idx * 3 + v); 
            } 
            // has normal attrib 
            if (mesh_attrib_flag & 0x0002) {
                // TODO: do correct transformation for normals and tangents that will work for any
                // transformation involving non-uniform scaling. For now, since I am not doing any scaling
                // so using the same modelView transform should be suffice for now
                // need to transform the normal here
                normals[v] = Math::normalize(activeShaderPtr_->transformNormal(mesh_manager.get_vn(mesh, f_idx * 3 + v)));
            } 
            // has vertex tangent
            if (mesh_attrib_flag & 0x0004) {
                tangents[v] = Math::normalize(activeShaderPtr_->transformTangent(mesh_manager.getTangent(mesh, f_idx * 3 + v)));
            }
        }

        // TODO: @ frustum culling
        // if projected triangle is partially out of screen, discard it for now
        
        // backface cull
         if (backfaceCulling(vertexPosView)) {
             continue;
         }
        // -------------
        // rasterization
        fillTriangle(vertexPosScreen, vertexPosView, normals, textureCoords, tangents);
        // ---------------------------------
    }
}

void Renderer::drawDebugLines(Mesh& mesh, Vec3<float>* vPosView, uint32_t f_idx) {
    // wireframe debugging
    drawTriangleWireFrame(Vec2<int>(vPosView[0].x, vPosView[0].y), 
                          Vec2<int>(vPosView[1].x, vPosView[1].y),
                          Vec2<int>(vPosView[2].x, vPosView[2].y));
    if (mesh.tangentBuffer) {
        for (int v = 0; v < 3; v++) {
            Vec3<float> vertexTangent = mesh_manager.getTangent(mesh, f_idx * 3 + v);
            drawTangents(mesh_manager.get_vertex(mesh, f_idx * 3 + v), vertexTangent);
        }
    }
    if (mesh.normal_buffer) {
        for (int v = 0; v < 3; v++) {
            Vec3<float> vertexNormal = mesh_manager.get_vn(mesh, f_idx * 3 + v);
            drawTangents(mesh_manager.get_vertex(mesh, f_idx * 3 + v), vertexNormal);
        }
    }
}

bool Renderer::depthTest(int x, int y, Vec3<float> baryCoord, Vec3<float>* vPosScreen) {
    int index = y * bufferWidth_ + x;
    float z = vPosScreen[0].z * baryCoord.x + vPosScreen[1].z * baryCoord.y + vPosScreen[2].z * baryCoord.z;
    if (z < zBuffer[index]) {
        zBuffer[index] = z;
        return true;
    }
    return false;
}

void Renderer::perspectiveCorrection(Vec3<float>* vPosView, Vec3<float>& baryCoord) {
    baryCoord.x /= vPosView[0].z;
    baryCoord.y /= vPosView[1].z;
    baryCoord.z /= vPosView[2].z;
    float pz = 1.f / (baryCoord.x + baryCoord.y + baryCoord.z); 
    baryCoord *= pz;
}

// TODO: Improve normal mapping
// TODO: Create multiple fragmentShader instance
// TODO: Maybe something like fragmentShaderPools[bufferWidth][bufferHeight]
void Renderer::fillTriangle(Vec3<float>* vPosScreen, Vec3<float>* vPosView, Vec3<float>* normals, Vec3<float>* textureCoords, Vec3<float>* tangents) {
    // TODO: @ Clean up using a determinant()
    Vec2<float> e1 = Vec2<float>(vPosScreen[1].x, vPosScreen[1].y) - Vec2<float>(vPosScreen[0].x, vPosScreen[0].y);
    Vec2<float> e2 = Vec2<float>(vPosScreen[2].x, vPosScreen[2].y) - Vec2<float>(vPosScreen[0].x, vPosScreen[0].y);
    float denom = e1.x * e2.y - e2.x * e1.y;
    // discard the triangle if its degenerated into a line
    if (denom == 0.f) {
        return;
    }

    float bbox[4] = {
        vPosScreen[0].x,
        vPosScreen[0].x,
        vPosScreen[0].y,
        vPosScreen[0].y,
    }; 

    Math::boundTriangle(vPosScreen, bbox, bufferWidth_, bufferHeight_);
    int xMin = bbox[0], xMax = bbox[1];
    int yMin = bbox[2], yMax = bbox[3];
    
    #pragma omp parallel for
    for (int y = yMin; y <= yMax; y++) {
        for (int x = xMin; x <= xMax; x++) {
            // compute barycentric coord
            Vec3<float> baryCoord = Math::barycentric(vPosScreen, x + .5f, y + .5f, denom); // overlapping test
            // TODO: @Work-around
            if (baryCoord.x < 0.f || baryCoord.y < 0.f  || baryCoord.z < 0.f) {
                continue;
            }
            // perspective correct interpolation of vertex attribs
            perspectiveCorrection(vPosView, baryCoord);
            // ------------------------------------------------------
            // depth test
            if (!depthTest(x, y, baryCoord, vPosScreen)) {
                continue;
            }
            // TODO: may not even need to bother checking
            // TODO: if normal is not provided, pass in interpolated normal
            // interpolate given vertex attribute
            uint32_t attribIdx = y * bufferWidth_ + x; 
            // TODO: @ pass in light to fragment shader
            if (mesh_attrib_flag & 0x0001) {
                activeShaderPtr_->fragmentAttribBuffer[attribIdx].textureCoord = Math::bary_interpolate(textureCoords, baryCoord);
            }
            // correct the interpolation for normal & tangents
            // note that the interpolation may make the length no longer unit 
            if (mesh_attrib_flag & 0x0002) {
                activeShaderPtr_->fragmentAttribBuffer[attribIdx].normal = Math::normalize(Math::bary_interpolate(normals, baryCoord));
            }
            if (mesh_attrib_flag & 0x0004) {
                activeShaderPtr_->fragmentAttribBuffer[attribIdx].tangent = Math::normalize(Math::bary_interpolate(tangents, baryCoord));
            }
            // Shading related
            // TODO: Maybe instead of interpolation, I can compute the fragment position in viewspace using depth
            activeShaderPtr_->lightParamsBuffer[attribIdx].viewSpaceFragmentPos = Math::bary_interpolate(vPosView, baryCoord);
            // compute fragment color
            Vec4<int> fragmentColor = activeShaderPtr_->fragmentShader(x, y);
            // write to backbuffer
            draw_pixel(x, y, fragmentColor);
            // -------------------
            // write to normal buffer
            normalBuffer[y * bufferWidth_ + x] = activeShaderPtr_->fragmentAttribBuffer[attribIdx].normal;
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

void Renderer::drawTriangleWireFrame(Vec2<int> V0, Vec2<int> V1, Vec2<int> V2) {
    drawLine(V0, V1);
    drawLine(V1, V2);
    drawLine(V2, V0);
}