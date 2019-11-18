#include "../include/Shader.h"
#include "../include/scene.h"

#define Ka 0.5f // Ambient coef
#define Kd 0.4f // Diffuse coef
#define Ks 0.1f // Specular coef

/// \TODO: Simplify logic and optimization
void DrawLine(Vec2<int> Start, Vec2<int> End, TGAImage& image, const TGAColor& color)
{
    int d = 1;
    //**** Divide by 0
    if (Start.x == End.x)
    {
        if (Start.y > End.y) Start.Swap(End);
        for (int i = Start.y; i <= End.y; i++)
        {
            image.set(Start.x, i, color);
        }

        return;
    }
    // Always start from left marching toward right
    // Swap Start and End
    if (Start.x > End.x) Start.Swap(End);
    // Derive the line function
    float Slope = (float)(Start.y - End.y) / (float)(Start.x - End.x);
    float Intercept = (float)End.y - Slope * End.x;
    Vec2<int> Next(Start);
    Vec2<int> StepA(1, 0);
    Vec2<int> StepB(1, 1);

    if (Slope < 0)
    {
        d *= -1;
        StepA.Swap(StepB);
    }
    if (Slope > 1 || Slope < -1)
    {
        StepA.Transpose();
        StepB.Transpose();
    }

    StepA.y *= d;
    StepB.y *= d;

    while (Next.x < End.x || Next.y != End.y)
    {
        //**** Slope < 1
        if (Slope >= -1 && Slope <= 1)
        {
            // Eval F(x+1,y+0.5)
            if (Next.y + d * 0.5f - Slope * (Next.x + 1.0f) - Intercept >= 0)
                Next += StepA;
            else
                Next += StepB;
        }
        /// \Bug: When slope is greater than 0, can invert x, y
        else
        {
            // Eval F(x+0.5, y+1)
            if (Next.y + d * 1.0f - Slope * (Next.x + .5f) - Intercept >= 0)
                Next += StepB;
            else
                Next += StepA;
        }
        //**** Draw pixel to the buffer
        image.set(Next.x, Next.y, color);
    }
}

void DrawTriangle(Vec2<int> V0, Vec2<int> V1, Vec2<int> V2, TGAImage& image, const TGAColor& color)
{
    DrawLine(V0, V1, image, color);
    DrawLine(V1, V2, image, color);
    DrawLine(V2, V0, image, color);
}

void Shader::BackfaceCulling()
{

}

bool FragmentShader::IsInShadow(Vec4<float> Fragment_Model, 
                                Mat4x4<float>& Transform, 
                                Mat4x4<float>& Viewport,
                                float* DepthBuffer)
{
    Vec4<float> Fragment_Shadow = Transform * Fragment_Model;
    Fragment_Shadow.x = Fragment_Shadow.x / Fragment_Shadow.w;
    Fragment_Shadow.y = Fragment_Shadow.y / Fragment_Shadow.w;
    Fragment_Shadow.w = 1.f;
    Vec4<float> Fragment_ShadowScreen = Viewport * Fragment_Shadow;
    
    if (Fragment_ShadowScreen.x > 799.f) Fragment_ShadowScreen.x = 799.f; 
    if (Fragment_ShadowScreen.y > 799.f) Fragment_ShadowScreen.y = 799.f; 

    int ShadowBuffer_x = Fragment_ShadowScreen.x;
    int ShadowBuffer_y = Fragment_ShadowScreen.y;

    float ShadowCoef = 0.1f; 

    // TODO: Using magic number 0.015f here to work around z-fighting for now
    if (Fragment_Shadow.z > DepthBuffer[ShadowBuffer_y * 800 + ShadowBuffer_x] + 0.015f)
        return true;

    return false;
}
/*
void Shader::DrawShadow(Model& Model, 
                        TGAImage& image, 
                        Vec3<float> LightPos, 
                        Vec3<float> LightDir, 
                        float* ShadowBuffer)
{
    // TODO: The refactor of Camera class may cause some bugs here
    // Do first pass rendering to decide which part of the mesh is visible
    Camera ShadowCamera;
    ShadowCamera.position = LightPos;
    // Need to negate the LightDir since LightDir reverted
    Mat4x4<float> View_Shadow = scene_manager.get_camera_view(ShadowCamera);
    // Cache the old MVP
    Mat4x4<float> Render_MVP = VS.MVP; 
    // same model, projection, viewport, but different view matrix
    FS.Shadow_MVP = VS.Projection * View_Shadow * VS.Model; 
    VS.MVP = FS.Shadow_MVP;

    Vec3<int>* IndexPtr = Model.Indices;
    int TriangleRendered = 0;

    while (TriangleRendered < Model.NumOfFaces)    
    {
        Vec4<float> V0_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[IndexPtr->x], 1.f);
        Vec4<float> V1_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[(IndexPtr + 1)->x], 1.f);
        Vec4<float> V2_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[(IndexPtr + 2)->x], 1.f);

        Vec3<float> V0_World(V0_World_Augmented.x, V0_World_Augmented.y, V0_World_Augmented.z);
        Vec3<float> V1_World(V1_World_Augmented.x, V1_World_Augmented.y, V1_World_Augmented.z);
        Vec3<float> V2_World(V2_World_Augmented.x, V2_World_Augmented.y, V2_World_Augmented.z);

        Vec3<float> V0V1 = V1_World - V0_World;
        Vec3<float> V0V2 = V2_World - V0_World;        

        Vec3<float> Surface_Normal = Math::Normalize(
                Math::CrossProduct(V0V1, V0V2));

        VS.Vertex_Shader(Model.VertexBuffer[IndexPtr->x],
                         Model.VertexBuffer[(IndexPtr + 1)->x],
                         Model.VertexBuffer[(IndexPtr + 2)->x], 
                         this->Triangle,
                         this->Triangle_Clip);

        Vec2<float> E1 = Triangle[1] - Triangle[0];
        Vec2<float> E2 = Triangle[2] - Triangle[0];
        float Denom = E1.x * E2.y - E2.x * E1.y;
        if (Denom == 0) continue;
        
        float bounds[4] = {
            Triangle[0].x, // Left
            Triangle[0].x, // Right
            Triangle[0].y, // Bottom
            Triangle[0].y  // Up
        };

        Math::bound_triangle(Triangle, bounds);

        for (int x = bounds[0]; x <= (int)bounds[1]; x++)
        {
            for (int y = bounds[2]; y <= (int)bounds[3]; y++)
            {
                Vec3<float> Weights = Math::barycentric(Triangle, x, y, Denom);
                // Point p is not inside of the triangle
                if (Weights.x < 0.f || Weights.y < 0.f || Weights.z < 0.f)
                    continue;

                float FragmentDepth = 0;
                
                // Depth test to see if current pixel is visible 
                if (this->FS.UpdateShadowBuffer(Triangle_Clip[0],      
                                                Triangle_Clip[1],
                                                Triangle_Clip[2],
                                                x, y, Weights, FragmentDepth))
                {
                    // TODO: Using magic number 1.f here
                    float Coef = (1.f - FragmentDepth) / 1.f;

                    // // Shade the fragment
                    // FS.Shadow_Shader(Vec2<int>(x, y), TGAColor(255 * Coef,
                    //                                            255 * Coef,
                    //                                            255 * Coef), image);
                }
            }
        }

        TriangleRendered++;
        IndexPtr += 3;
    }

    VS.MVP = Render_MVP;
}

void Shader::draw_cubemap(Cubemap& cubemap, TGAImage& image, struct Camera& camera) {

}

bool UpdateDepthBuffer(Vec3<float> V0, 
                       Vec3<float> V1, 
                       Vec3<float> V2, 
                       int ScreenX, 
                       int ScreenY, 
                       Vec3<float> Weights,
                       float& FragmentDepth,
                       float* DepthBuffer)
{
    int Index = ScreenY * 800 + ScreenX;
    // TODO: Do the perspective correct interpolation here
    FragmentDepth = V0.z * Weights.z + V1.z * Weights.x + V2.z * Weights.y;

    if (FragmentDepth < DepthBuffer[Index]) 
    {
        DepthBuffer[Index] = FragmentDepth;
        return true;
    }

    return false;
}


// TODO: change the internal representation of u,v,w so that u is for v0, v is for v1, and w is for v2
// Generate the occlusion texture
void Shader::DrawOcclusion(Model& Model, TGAImage& occlusion_texture, float* occlusion_depth_buffer)
{
    Vec3<int>* IndexPtr = Model.Indices;
    int TriangleRendered = 0;

    while (TriangleRendered < Model.NumOfFaces)    
    {
        VS.Vertex_Shader(Model.VertexBuffer[IndexPtr->x],      
                         Model.VertexBuffer[(IndexPtr + 1)->x],
                         Model.VertexBuffer[(IndexPtr + 2)->x],
                         this->Triangle,
                         this->Triangle_Clip);

        // in screen space
        // (V1.x - V0.x) * (V2.y - V0.y) == (V2.x - V0.x) * (V1.y - V0.y)
        Vec2<float> E1 = Triangle[1] - Triangle[0];
        Vec2<float> E2 = Triangle[2] - Triangle[0];
        float Denom = E1.x * E2.y - E2.x * E1.y;
        if (Denom == 0) return;
        
        float bounds[4] = {
            Triangle[0].x, // Bottom
            Triangle[0].x, // Top
            Triangle[0].y, // Left
            Triangle[0].y  // Right
        };

        Math::bound_triangle(Triangle, bounds);

        for (int x = bounds[0]; x <= (int)bounds[1]; x++)
        {
            for (int y = bounds[2]; y <= (int)bounds[3]; y++)
            {
                Vec3<float> Weights = Math::barycentric(Triangle, x, y, Denom);
                if (Weights.x < 0.f || Weights.y < 0.f || Weights.z < 0.f) continue;
                float fragment_depth = 0;
                // Depth test to see if current pixel is visible 
                if (UpdateDepthBuffer(Triangle_Clip[0],
                                      Triangle_Clip[1],
                                      Triangle_Clip[2],
                                      x, y, Weights,
                                      fragment_depth,
                                      occlusion_depth_buffer))
                {
                    Vec2<float> uv0 = Model.TextureBuffer[IndexPtr->y];
                    Vec2<float> uv1 = Model.TextureBuffer[(IndexPtr + 1)->y];
                    Vec2<float> uv2 = Model.TextureBuffer[(IndexPtr + 2)->y];
                    Vec2<float> uv;  
                    uv.x = Weights.z * uv0.x + Weights.x * uv1.x + Weights.y * uv2.x;
                    uv.y = Weights.z * uv0.y + Weights.x * uv1.y + Weights.y * uv2.y;
                    occlusion_texture.set(uv.x * 1024, uv.y * 1024, TGAColor(
                                255, 
                                255, 
                                255));
                }
            }
        }

        IndexPtr += 3;    
        TriangleRendered++;
    }
}
*/

Shader_Base::Shader_Base() {
    vertexAttribFlag = 1;
    texture_ = nullptr;
    normalMap_ = nullptr;
    fragmentAttribBuffer = nullptr;
    lightingParamBuffer = nullptr;
}

void Shader_Base::initFragmentAttrib(uint32_t bufferWidth, uint32_t bufferHeight) {
    bufferWidth_ = bufferWidth;
    bufferHeight_ = bufferHeight;
    fragmentAttribBuffer = new FragmentAttrib[bufferWidth * bufferHeight];
    lightingParamBuffer = new LightingParams[bufferWidth * bufferHeight];
}

// TODO: Perspective correct texture sampling
Vec3<float> Shader_Base::sampleTexture2D(Texture& texture, float u, float v) {
    Vec3<float> res;
    uint32_t samplePosX = std::fmod(u, 1.f) * texture.textureWidth;
    // 1 - v here because the texture image coord y starts at the top while
    // texture coord y starts from bottom, thus flip vertically 
    uint32_t samplePosY = (1 - std::fmod(v, 1.f)) * texture.textureHeight;
    uint32_t pixelIdx = texture.textureWidth * samplePosY + samplePosX;
    res.x = (float)(texture.pixels[pixelIdx * texture.numChannels]);     // R
    res.y = (float)(texture.pixels[pixelIdx * texture.numChannels + 1]); // G
    res.z = (float)(texture.pixels[pixelIdx * texture.numChannels + 2]); // B
    return res;
}

Vec3<float> Shader_Base::sampleNormal(Texture& normalMap, float u, float v) {
    Vec3<float> res;
    uint32_t samplePosX = u * normalMap.textureWidth;
    // 1 - v here because the texture image coord y starts at the top while
    // texture coord y starts from bottom, thus flip vertically 
    uint32_t samplePosY = (1 - v) * normalMap.textureHeight;
    uint32_t pixelIdx = normalMap.textureWidth * samplePosY + samplePosX;
    res.x = normalMap.pixels[pixelIdx * normalMap.numChannels];     // R
    res.y = normalMap.pixels[pixelIdx * normalMap.numChannels + 1]; // G
    res.z = normalMap.pixels[pixelIdx * normalMap.numChannels + 2]; // B
    // normalize the component value from [0, 255] to [-1, 1]
    res.x = (float)res.x / 255.f * 2 - 1.f; 
    res.y = (float)res.y / 255.f * 2 - 1.f; 
    res.z = (float)res.z / 255.f * 2 - 1.f; 
    // Do I need to normalize the remapped result here?
    return res;
}

void Shader_Base::set_model_matrix(Mat4x4<float>& model) {
    model_ = model;
}

void Shader_Base::set_view_matrix(Mat4x4<float>& view) {
    view_ = view;
}

void Shader_Base::set_projection_matrix(Mat4x4<float>& projection) {
    projection_ = projection;
}

Vec3<float> Shader_Base::transformNormal(Vec3<float>& normal) {
    Vec4<float> result = view_ * model_ * Vec4<float>(normal, 0.f);
    return Vec3<float>(result.x, result.y, result.z);
}

Vec3<float> Shader_Base::transformTangent(Vec3<float>& tangent) {
    Vec4<float> result = view_ * model_ * Vec4<float>(tangent, 0.f);
    return Vec3<float>(result.x, result.y, result.z);
}

void Shader_Base::bindTexture(Texture* texture) {
    texture_ = texture;
}

void Shader_Base::unbindTexture() {
    texture_ = nullptr;
}

void Shader_Base::clearFragmentAttribs() {
    for (int fragment = 0; fragment < bufferHeight_ * bufferWidth_; fragment++) {
        fragmentAttribBuffer[fragment] = { };
        // TODO: do i need to refresh lighting params as well? probably not
    }
}

Vec4<float> SkyboxShader::vertex_shader(Vec3<float>& v) {
    Vec4<float> glPosition = projection_ * view_ * Vec4<float>(v, 1.f);
    glPosition.z = glPosition.w;
    return glPosition;
}

// TODO: bug here, part of the cube is not rendered for some reason
Vec4<int> SkyboxShader::fragment_shader(int x, int y) {
    Vec4<int> sampleColor(100, 100, 100, 255);
    // sample based on direction
    return sampleColor;
}

Phong_Shader::Phong_Shader() {

}

Vec4<float> Phong_Shader::vertex_shader(Vec3<float>& v) {
    return projection_ * view_ * model_ * Vec4<float>(v, 1.f);    
}

// Fragment normal
// Fragment textureUV
// Fragment lightDirection
// Fragment viewDirection
// TODO: gamma-correction
// TODO: specular mapping
// TODO: if a given mesh does not come with vertex normal
//     : generate normals at loading and feed interpolated vn in here instead
//     : of using normal map
Vec4<int> Phong_Shader::fragment_shader(int x, int y) {
    FragmentAttrib& attribs = fragmentAttribBuffer[y * bufferWidth_ + x];
    LightingParams& lightingParams = lightingParamBuffer[y * bufferWidth_ + x];
    Vec3<float> lightDirection = lightingParams.direction;
    Vec3<float> lightColor = lightingParams.color;
    Vec3<float> diffuseSample, ambient, diffuse, specular;

    Vec4<float> result(0.f, 0.f, 0.f, 255.f);
    // default diffuse color
    diffuseSample = Vec3<float>(230.f, 154.f, 222.f);
    if (texture_) {
        diffuseSample = sampleTexture2D(*texture_, attribs.textureCoord.x, attribs.textureCoord.y);
        // Gamma-correction at gamma of 2.0
        diffuseSample.x *= diffuseSample.x;
        diffuseSample.y *= diffuseSample.y;
        diffuseSample.z *= diffuseSample.z;
    }
    Vec3<float> normal = attribs.normal;
    if (normalMap_) { 
        Vec3<float> sampledNormal = sampleNormal(*normalMap_, attribs.textureCoord.x, attribs.textureCoord.y); 
        // TODO: Need to consider the handed-ness of the tangent space where normals are defined in
        Vec3<float> interpolatedNormal = attribs.normal;
        Vec3<float> tangent = attribs.tangent;
        // Gram-Schemidt process to re-orthoganize the tangent and fragmentNormal
        tangent = Math::Normalize(tangent - interpolatedNormal * Math::DotProduct_Vec3(tangent, interpolatedNormal));
        Vec3<float> biTangent = Math::CrossProduct(tangent, interpolatedNormal);
        normal.x = tangent.x * sampledNormal.x + biTangent.x * sampledNormal.y + interpolatedNormal.x * sampledNormal.z;
        normal.y = tangent.y * sampledNormal.x + biTangent.y * sampledNormal.y + interpolatedNormal.y * sampledNormal.z;
        normal.z = tangent.z * sampledNormal.x + biTangent.z * sampledNormal.y + interpolatedNormal.z * sampledNormal.z;
    } 
    // Assuming that lightDirection is already normalized
    float diffuseCoef = Math::DotProduct_Vec3(normal, lightDirection);
    diffuseCoef = Math::clamp_f(diffuseCoef, 0.f, 1.f);
    if (normalMap_) {
        diffuse = diffuseSample * diffuseCoef;
    } else {
        diffuse = diffuseSample;
    }
    result = Vec4<float>(ambient + diffuse, 0.f);
    // invert gamma-correction
    if (texture_) {
        result.x = sqrt(result.x);
        result.y = sqrt(result.y);
        result.z = sqrt(result.z);
    }
    Vec4<int> fragmentColor((int)result.x, (int)result.y, (int)result.z, (int)result.w);
    return fragmentColor;
}
