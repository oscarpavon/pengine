#include "model.h"

#define CGLTF_IMPLEMENTATION
#include "third_party/cgltf.h"
#include "stdio.h"
#include <vec3.h>

#include "file_loader.h"

#include "engine.h"

cgltf_data* current_data;
cgltf_animation* current_animation;
cgltf_animation_channel* current_channel;
cgltf_animation_sampler* current_sampler;
bool copy_nodes = false;

Array* actual_vertex_array;
Array* actual_index_array;
Array* current_array;
struct Model* actual_model;
bool model_loaded = false;
int models_parsed = 0;

Node* get_node_by_name(Array* array, const char* name){
  for( int i = 0; i < array->count ; i++ ){
    Node* node = get_from_array(array,i);
    if( strcmp( node->name , name ) == 0){
      return node;
    }
  }
}

void read_accessor_indices(cgltf_accessor* accessor){
  init_array(actual_index_array,sizeof(unsigned short int),accessor->count);
  for(size_t i = 0 ; i < accessor->count ; i++){
      unsigned short int index = cgltf_accessor_read_index(accessor,i);
      add_to_array(actual_index_array,&index);
  }
}
/*Read accesor and allocated data in current_array or actual_vertex_array */
void read_accessor(cgltf_accessor* accessor){
  switch (accessor->type)
  {
  case cgltf_type_vec2:
    for(size_t i = 0 ; i < accessor->count ; i++){
        Vertex* vertex = get_from_array(actual_vertex_array,i);
        cgltf_accessor_read_float(accessor, i, &vertex->uv[0], 2);
    }
    break;
  case cgltf_type_vec3:
    init_array(actual_vertex_array,sizeof(Vertex),accessor->count);
    for(size_t v = 0 ; v < accessor->count ; v++){
      struct Vertex vertex;
      memset(&vertex,0,sizeof(struct Vertex));
      cgltf_accessor_read_float(accessor,v,&vertex.postion[0],3);
      add_to_array(actual_vertex_array,&vertex);
    }
    break;
  case cgltf_type_vec4:
    
    for(int i = 0 ; i < accessor->count ; i++){
      vec4 quaternion;      
      cgltf_accessor_read_float(accessor, i, quaternion , 4);
      for(int j = 0; j < 4 ; j++){
        float number = (float)quaternion[j];
        int exponent;
        frexp(number,&exponent);
        float new_float = (float)ldexp(number,exponent);
        LOG("Number: %f\n",number);
      }
      add_to_array(current_array,quaternion);
    }
    vec4 test1;
    vec4 test2;
    glm_vec4_copy((vec4){1,1,1,1},test1);
    glm_vec4_copy((vec4){2,-4.4264304e-17,2,2},test2);

    //add_to_array(current_array,test1);
    //add_to_array(current_array,test2);
    
    for(int j = 0; j < current_array->count ; j++){
        float* quaternion = get_from_array(current_array,j);
        glm_vec4_print(quaternion,stdout);
    }  
    
    break;
  case cgltf_type_scalar:
    
    for(int i = 0 ; i < accessor->count ; i++){
      float number;      
      cgltf_accessor_read_float(accessor, i, &number, 1);
      add_to_array(current_array,&number);
    }
    
    break;
  default:
    break;
  }

  switch (accessor->component_type)
  {
  case cgltf_component_type_r_16:
    /* code */
    break;
  
  default:
    break;
  }
}
void load_attribute(cgltf_attribute* attribute){
  switch (attribute->type)
  {
  case cgltf_attribute_type_position:
    read_accessor(attribute->data);
    break;
  case cgltf_attribute_type_texcoord:
    read_accessor(attribute->data);
    break;
  
  default:
    break;
  }

  if(attribute->data->has_min){
    
    glm_vec3_copy(attribute->data->min,actual_model->min);
    
  }
  if(attribute->data->has_max){
    
    glm_vec3_copy(attribute->data->max,actual_model->max);
    
  }
}


void load_primitive(cgltf_primitive* primitive){
  
  for(unsigned short int i = 0; i < primitive->attributes_count; i++){
    load_attribute(&primitive->attributes[i]);
  }
  
  read_accessor_indices(primitive->indices);
}

void load_mesh(cgltf_mesh* mesh){
  new_empty_model();
  actual_vertex_array = &selected_model->vertex_array;
  actual_index_array = &selected_model->index_array;
  actual_model = selected_model;

  for(int i = 0; i < mesh->primitives_count ; i++){
    load_primitive(&mesh->primitives[i]);
  }  
  model_loaded = true;
}

int load_node(Node* parent, cgltf_node *in_cgltf_node, Node* store_nodes, int index_to_store){

  if(copy_nodes){
    Node new_node;
    memset(&new_node,0,sizeof(Node));  

    if(in_cgltf_node->parent && parent != NULL)
      new_node.parent = parent;  

    strcpy(new_node.name,in_cgltf_node->name);

    memcpy(new_node.translation,in_cgltf_node->translation,sizeof(vec3));
    memcpy(new_node.rotation, in_cgltf_node->rotation, sizeof(vec4));

    add_to_array(&model_nodes,&new_node);
  }

  if(in_cgltf_node->mesh != NULL)
    load_mesh(in_cgltf_node->mesh);

  if(in_cgltf_node->skin != NULL){    
    current_nodes_array = &model_nodes;
    current_loaded_component_type = COMPONENT_SKINNED_MESH; 
    LOG("Nodes assigned to current_nodes_array\n");
  }
  
  Node* loaded_parent = get_from_array(&model_nodes,model_nodes.count-1);
  if(in_cgltf_node->children_count == 0 && in_cgltf_node->mesh == NULL)
    return 1;

  int offset = 0;
  for(int i = 0; i < in_cgltf_node->children_count; i++){ 
    offset = load_node( loaded_parent , in_cgltf_node->children[i] , store_nodes , index_to_store + (i+1+offset) );
  }

}

void check_LOD(cgltf_data* data){
  cgltf_mesh* meshes[4];
  memset(meshes,0,sizeof(meshes));

  HierarchicalLevelOfDetail* hirarchical_level_of_detail;
  if(data->nodes_count > 1){
    for(int i = 0; i < data->nodes_count; i++){
      int node_name_size = strlen(data->nodes[i].name);
      char name[node_name_size];
      strcpy(name,data->nodes[i].name);    

      for(int n = 0; n<node_name_size; n++){
        if(name[n] == '_'){
          if(strcmp("LOD0",&name[n]+1) == 0){
            

            LOG("Found LOD0\n");
            meshes[models_parsed] = data->nodes[i].mesh;
            models_parsed++;
            
            break;
          }
          if(strcmp("LOD1",&name[n]+1) == 0){
            LOG("Found LOD1\n");
            meshes[models_parsed] = data->nodes[i].mesh;
            models_parsed++;

            break;
          }
          
          if(strcmp("LOD2",&name[n]+1) == 0){
            LOG("Found LOD2\n");
            meshes[models_parsed] = data->nodes[i].mesh;
            models_parsed++;
            break;
          }
          if(strcmp("HLOD",&name[n]+1) == 0){
            HierarchicalLevelOfDetail new_hirarchical;
            memset(&new_hirarchical,0,sizeof(HierarchicalLevelOfDetail));
            memset(&new_hirarchical.model,0,sizeof(Model));
            actual_vertex_array = &new_hirarchical.model.vertex_array;
            actual_index_array = &new_hirarchical.model.index_array;
            actual_model = &new_hirarchical.model;

            //load_mesh(data->nodes[i].mesh);
            init_model_gl_buffers(&new_hirarchical.model);
            new_hirarchical.model.shader = create_engine_shader(standart_vertex_shader,standart_fragment_shader);


            add_to_array(&array_hirarchical_level_of_detail,&new_hirarchical);
            hirarchical_level_of_detail = 
            get_from_array(&array_hirarchical_level_of_detail,
            array_hirarchical_level_of_detail.count-1);
          }
        }
      }
    }
   
  }
  if(models_parsed > 1){

    for(int i = 0; i< models_parsed; i++){
      load_mesh(meshes[i]);
      init_model_gl_buffers(selected_model);
    }
  }
}

void load_current_sampler_to_channel(AnimationChannel* channel){
  AnimationSampler sampler;
  memset(&sampler,0,sizeof(AnimationSampler));
  init_array(&sampler.inputs,sizeof(float),current_sampler->input->count);
  init_array(&sampler.outputs_vec4,sizeof(float)*4,current_sampler->output->count);
  current_array = &sampler.inputs;
  read_accessor(current_sampler->input);
  current_array = &sampler.outputs_vec4;
  read_accessor(current_sampler->output);
  memcpy(&channel->sampler,&sampler,sizeof(AnimationSampler));
}




void load_current_channel_to_animation(Animation* animation){
  AnimationChannel channel;
  memset(&channel,0,sizeof(AnimationChannel));
  channel.node = get_node_by_name(&model_nodes,current_channel->target_node->name);
  switch (current_channel->target_path)
  {
  case cgltf_animation_path_type_rotation:
    channel.path_type = PATH_TYPE_ROTATION; 
    break;
  case cgltf_animation_path_type_translation:
    channel.path_type = PATH_TYPE_TRANSLATION; 
    break;

  default:
    break;
  }
  
  current_sampler = current_channel->sampler;
  load_current_sampler_to_channel(&channel);
   
  add_to_array(&animation->channels,&channel);
}



void load_current_animation(){
  Animation new_animation;
  memset(&new_animation,0,sizeof(Animation));
  strcpy(new_animation.name,current_animation->name);
  init_array(&new_animation.channels,sizeof(AnimationChannel),current_animation->channels_count);
  for(int i = 0; i< current_animation->channels_count ; i++){
    current_channel = &current_animation->channels[i];
    load_current_channel_to_animation(&new_animation);
  }
  add_to_array(&model_animation,&new_animation);
}

int load_model(const char* path){
  memset(&model_animation,0,sizeof(Array));
  memset(&model_nodes,0,sizeof(Array));

  model_loaded = false;
  File new_file;

  load_file(path,&new_file);

  cgltf_options options = {0};
  cgltf_data* data = NULL;

  cgltf_result result = cgltf_parse(&options,new_file.data,new_file.size_in_bytes, &data);

  if (result != cgltf_result_success){
    LOG("Model no loaded: %s \n", new_file.path);
    return -1;
  }
  current_data = data;

  result = cgltf_load_buffers(&options,data,new_file.path);
  if (result != cgltf_result_success){
    LOG("Buffer no loaded: %s \n", new_file.path);
    return -1;
  }
  close_file(&new_file);

  current_loaded_component_type = STATIC_MESH_COMPONENT;
  check_LOD(data);

  if(model_loaded){
    LOG("gltf loaded with LODs. \n");
 
    cgltf_free(data);
    
    int model_result = models_parsed;
    models_parsed = 0;
    return model_result;
  }

  if(data->skins_count >= 1){
    init_array(&model_nodes,sizeof(Node),data->nodes_count+1);
    memset(model_nodes.data,0,sizeof(Node) * data->nodes_count);
    copy_nodes = true; 
  }

  for(int i = 0; i < data->scene->nodes_count ; i++){
    load_node(NULL, data->scene->nodes[i],(Node*)model_nodes.data,0);
  }
  
  /* NULL vertex/index array beacouse not needed anymore */
  actual_vertex_array = NULL;
  actual_index_array = NULL;

  if(data->animations_count >= 1){
    init_array(&model_animation,sizeof(Animation),data->animations_count);
    for(int i = 0; i < data->animations_count; i++){
      current_animation = &data->animations[i];
      load_current_animation();
    }
  }
  models_parsed++;
  LOG("gltf loaded: %s. \n",path);

  cgltf_free(data);
  current_data = NULL;
 
  
  actual_model = NULL;

  int model_result = models_parsed;
  models_parsed = 0;
  return model_result;
}

