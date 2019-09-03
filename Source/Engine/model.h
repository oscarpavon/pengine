#ifndef MODEL_H
#define MODEL_H

#include "utils.h"

#include <GLES2/gl2.h>

#include "../ThirdParty/cglm/cglm.h"

#include "images.h"

#include "skeletal.h"

#include "array.h"

typedef struct Model{
    int id;
    Array vertex_array;
    Array index_array;
    vec3 min;
    vec3 max;
    
    GLuint vertex_buffer_id;
    GLuint index_buffer_id;

    mat4 model_mat;
    GLuint shader;

    Texture texture;

}Model;

struct LoadGeometry{
    Array vertex_array;
    Array index_array;
};

typedef struct DrawData{
    u32 shader;
    u32 texture;
    u32 vertex;
    u32 index;
}DrawData;

int load_model(const char* path);

int model_load_from_content(void* gltf_data, u32 size);

Array model_nodes;
Array model_animation;
mat4 inverse_bind_matrices[35];

#endif // !MODEL_H