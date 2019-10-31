#include <string>

struct Mesh_Instance {
    uint32_t mesh_id;
    uint32_t instance_id;
};

class Mesh {
public:
    std::string name;
    float* vertex_buffer;
    float* texture_uv_buffer;
    float* normal_buffer;
    unsigned int* indices; // do not need index buffer if rearranging the vertex

    int num_vertices, num_faces, num_texture_coord, num_normal;
    int material_id;

    Mesh();
    void load_obj(const char* filename);
    void load_texture(const char* filename);
};