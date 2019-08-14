#include "HLOD_factory.h"
#include "../editor.h"

#include "../Textures/texture_factory.h"
#include <math.h>

vec2 UV_tranlation_offset;
void* saved_vertex_data[6];
int saved_vertex_data_count = 0;
Model* saved_vertex_model[6];
int saved_vertex_model_count = 0;

HLODBoxComponent* current_HLOD_box_component;


void check_is_inside(ComponentDefinition* component_definition){
    
    if(component_definition->type == STATIC_MESH_COMPONENT){
        StaticMeshComponent* mesh = component_definition->data;
        
        if(glm_aabb_contains(current_HLOD_box_component->bounding_box,mesh->bounding_box)){
            
            add_to_array(&array_elements_for_HLOD_generation,&component_definition->parent);
            LOG("%s\n",component_definition->parent->name);
            unsigned int* id = get_from_array(&mesh->meshes,mesh->meshes.count-1);
            Model* model = get_from_array(actual_model_array,*id);
             
            Model new_model;
            memset(&new_model,0,sizeof(Model));
            init_array(&new_model.vertex_array,sizeof(Vertex),model->vertex_array.count);
            void* data = new_model.vertex_array.data;
            duplicate_model_data(&new_model,model);

            new_model.vertex_array.data = data;
            memcpy(new_model.vertex_array.data,model->vertex_array.data,model->vertex_array.actual_bytes_size);
           
            translate_UV(VEC3(UV_tranlation_offset[0],UV_tranlation_offset[1],0),&new_model,(vec2){0,0});
            UV_tranlation_offset[0] += 1;
            //UV_tranlation_offset[1] = 1;

            scale_UV(0.5, &new_model,(vec2){0,0});

            saved_vertex_model[saved_vertex_model_count] = model;
            saved_vertex_model_count++;
            saved_vertex_data[saved_vertex_data_count] = model->vertex_array.data;
            saved_vertex_data_count++;
            

            model->vertex_array.data = new_model.vertex_array.data;
        }
    }
}

void merge_sphere_to_cluster(Sphere* sphere, Sphere* sphere2){

}

bool sphere01_inside_sphere02(Sphere* sphere01, Sphere* sphere02){
    vec3 distance;
    glm_vec3_sub(sphere02->center,sphere01->center,distance);
    float easy_distance = glm_vec3_dot(distance,distance);

    float sum_radius_squared = sphere01->radius + sphere02->radius;
    sum_radius_squared *= sum_radius_squared;
    
    if(easy_distance+sphere01->radius <= sphere02->radius)
        return true;
}

inline static float get_sphere_volume(Sphere* sphere){
    if(sphere->volume == 0){
        float volume = ( (4.f/3.f) * M_PI * ( sphere->radius * sphere->radius * sphere->radius) );
        sphere->volume = volume;
        return volume;
    }
    else
        return sphere->volume;
}

float sphere_volume_overlap(Sphere* sphere01, Sphere* sphere02){
    if( sphere01_inside_sphere02(sphere01,sphere02) )
        return get_sphere_volume(sphere01);
    
    if( sphere01_inside_sphere02(sphere02, sphere01 ) )
        return get_sphere_volume(sphere02);

}

float calculate_fill_factor(Sphere* sphere01 , Sphere* sphere02, float fill_factor_sphere01, float fill_factor_sphere02){
    float overlap_volume = sphere_volume_overlap(sphere01,sphere02);
    return ( fill_factor_sphere01 * get_sphere_volume(sphere01) + 
            fill_factor_sphere02 * get_sphere_volume(sphere02) - overlap_volume) / 
            (get_sphere_volume(sphere01) + get_sphere_volume(sphere02));
}

int short_cluster(const void* cluster01 , const void* cluster02){
    HLODCluster cluster;
    cluster.cost = ((HLODCluster*)cluster01)->cost - ((HLODCluster*)cluster02)->cost;
    return ((HLODCluster*)cluster01)->cost - ((HLODCluster*)cluster02)->cost;
   
}

void compute_bounding_sphere_for_every_mesh(){
   for(int i = 0; i < actual_elements_array->count ; i++){
        Element* element01 = get_from_array(actual_elements_array,i);
        StaticMeshComponent* mesh01 = get_component_from_element(element01,STATIC_MESH_COMPONENT);
        if(!mesh01)
            continue;

        for(int j = i+1; j<actual_elements_array->count  ; j++){
            Element* element02 = get_from_array(actual_elements_array,j);
            StaticMeshComponent* mesh02 = get_component_from_element(element02,STATIC_MESH_COMPONENT);
            if(!mesh02)
                continue;

            HLODCluster cluster;
            memset(&cluster, 0, sizeof(HLODCluster));
            cluster.is_valid = true;

            init_array(&cluster.elements,sizeof(StaticMeshComponent*),8);
            add_to_array(&cluster.elements, &mesh01);
            add_to_array(&cluster.elements,&mesh02);

            Sphere sphere01;
            Sphere sphere02;
            memset(&sphere01,0,sizeof(Sphere));
            memset(&sphere02,0,sizeof(Sphere));
            
            sphere01.radius = glm_aabb_radius(mesh01->bounding_box);
            glm_vec3_copy(mesh01->center,sphere01.center);

            sphere02.radius = glm_aabb_radius(mesh02->bounding_box);
            glm_vec3_copy(mesh02->center,mesh02->center);

            Sphere cluster_sphere; 
            cluster_sphere.radius = sphere01.radius + sphere02.radius;

            cluster.bounding_sphere = cluster_sphere;

            cluster.fill_factor = calculate_fill_factor(&sphere01,&sphere02,1.f,1.f);
            cluster.cost = (cluster.bounding_sphere.radius * cluster.bounding_sphere.radius * cluster.bounding_sphere.radius ) / cluster.fill_factor;
            if(cluster.cost <= ( (1000 * 1000 * 1000) / 50) ){
                add_to_array(&HLOD_generated_cluster,&cluster);
            }
        }
    }

    for(int i = 0; i<HLOD_generated_cluster.count ; i++){
        HLODCluster* cluster = get_from_array(&HLOD_generated_cluster,i);
        LOG("Before short: %f\n",cluster->cost);
    }

    qsort(HLOD_generated_cluster.data,HLOD_generated_cluster.count,sizeof(HLODCluster),short_cluster);
    
    for(int i = 0; i<HLOD_generated_cluster.count ; i++){
        HLODCluster* cluster = get_from_array(&HLOD_generated_cluster,i);
        LOG("After short: %f\n",cluster->cost);
    }

    for(int i = 0; i<HLOD_generated_cluster.count ; i++){
        HLODCluster* cluster = get_from_array(&HLOD_generated_cluster,i);
        if(cluster->is_valid){
            for(int j = 0; j<i ; j++){
                HLODCluster* cluster_for_merge = get_from_array(&HLOD_generated_cluster,j);
                if(cluster_for_merge->is_valid){

                    for(int e = 0; e < cluster_for_merge->elements.count; e++){
                        Element** ppElement = get_from_array(&cluster_for_merge->elements,e);
                        Element* element = ppElement[0];
                        for(int h = 0; h<cluster->elements.count; i++){
                            Element** ppElement2 = get_from_array(&cluster->elements,h);
                            Element* element2 = ppElement2[0];
                            if(element->id == element2->id)
                            {

                                HLODCluster new_cluster;
                                memset(&new_cluster,0,sizeof(HLODCluster));
                                new_cluster.is_valid = true;
                                Sphere new_bounding_sphere;
                                new_bounding_sphere.radius = cluster->bounding_sphere.radius + cluster_for_merge->bounding_sphere.radius;
                                new_bounding_sphere.volume = cluster->bounding_sphere.volume + cluster_for_merge->bounding_sphere.volume;
                                new_cluster.bounding_sphere = new_bounding_sphere;

                                //merge
                                new_cluster.fill_factor = calculate_fill_factor(&cluster->bounding_sphere,&cluster_for_merge->bounding_sphere,cluster->fill_factor,cluster_for_merge->fill_factor);
                                new_cluster.cost = (new_cluster.bounding_sphere.radius * new_cluster.bounding_sphere.radius * new_cluster.bounding_sphere.radius ) / new_cluster.fill_factor;
                                if(new_cluster.cost <= ( (1000 * 1000 * 1000) / 50) ){
                                    cluster_for_merge->fill_factor = new_cluster.fill_factor;
                                    cluster_for_merge->bounding_sphere = new_cluster.bounding_sphere;
                                    cluster_for_merge->cost = new_cluster.cost;
                                    cluster->is_valid = false;
                                    break;
                                }

                                
                            }
                        }

                        
                    }
                    
                }
            }
        }
    }

    for(int i = 0; i<HLOD_generated_cluster.count ; i++){
        HLODCluster* cluster = get_from_array(&HLOD_generated_cluster,i);
        if(cluster->is_valid){
            LOG("Cluster active, %i\n",i);
        }
    }
}


void generate_HLODS(){
    UV_tranlation_offset[0] = 0;
    UV_tranlation_offset[1] = -2;
    if(!array_elements_for_HLOD_generation.initialized){
        init_array(&array_elements_for_HLOD_generation,sizeof(Element*),30);
        array_elements_for_HLOD_generation.isPointerToPointer = true;
    }
    clean_array(&array_elements_for_HLOD_generation);

    for(int i = 0; i< HLOD_cluster_count; i++){
       Element* elmeent = HLOD_clusters[i];
       current_HLOD_box_component = get_component_from_element(elmeent,COMPONENT_HLOD_BOX);
       for_each_element_components(check_is_inside);
    }   
    
    if(!HLOD_generated_cluster.initialized){
        init_array(&HLOD_generated_cluster, sizeof(HLODCluster),8);
    }
    
    compute_bounding_sphere_for_every_mesh();


    if(export_gltf("../assets/HLOD/out.gltf") == -1){
        LOG("Not exported\n");
        return;
    }

    saved_vertex_model[0]->vertex_array.data = saved_vertex_data[0];
    saved_vertex_model[1]->vertex_array.data = saved_vertex_data[1];
    
    merge_textures();

   
    //system("blender --python ../scripts/Blender/import.py");

    editor_add_HLOD_element();

}