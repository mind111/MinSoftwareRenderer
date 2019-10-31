#include "Shader.h"
#include "scene.h"

class Renderer {
public:
    void draw_scene(Scene& scene);
private:
    void draw_mesh();
};