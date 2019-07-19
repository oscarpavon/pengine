#ifndef ENGINE_DATA_TYPES
#define ENGINE_DATA_TYPES

typedef struct TransformComponent TransformComponent;
typedef struct ComponentDefinition ComponentDefinition;

typedef enum ElementType{
    ELEMENT_TYPE_CAMERA = 1,
    ELEMENT_TYPE_PLAYER_START = 2,
    ELEMENT_TYPE_COLLIDER = 3,
    ELEMENT_TYPE_MODEL = 4,
    ELEMENT_TYPE_PLAYER_CONTROLLER = 5
}ElementType;

typedef struct Element{
    unsigned int id;
    ElementType type;
    char name[20];
    vec3 position;
    versor rotation;
    struct Model* model;
    char model_path[20];
    char texture_path[20];
    short int duplicated_of_id;
    bool selected;
    bool has_HLOD;
    bool has_LOD;
    unsigned int model_id;
    unsigned int components_count;
    Array components;
    TransformComponent* transform;
}Element;

typedef struct PlayerStart{
    vec3 position;
    versor rotation; 
}PlayerStart;

typedef enum DataType{
    DATA_TYPE_LEVEL = 0,
    DATA_TYPE_GUI = 1,
    DATA_TYPE_HEADER = 2
}DataType;

typedef void(*Action)(void);

typedef struct ActionPointer{
    int id;
    Action action;
}ActionPointer;

#endif // !ENGINE_DATA_TYPES
