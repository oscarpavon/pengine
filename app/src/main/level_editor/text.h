#ifndef EDITOR_TEXT_H
#define EDITOR_TEXT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdbool.h>
#include "../vector.h"

void init_text_renderer();
void render_text(const char *text, float x, float y, float sx, float sy , bool mark);
void draw_directory_files();

void text_renderer_loop();

FT_Face face;
static inline void set_text_size(float size){
    FT_Set_Pixel_Sizes(face, 0, size);  
}


typedef struct{
    bool selected;
    const char* text;    
}TextElement;

typedef struct {
    int count;    
    TextElement *elements;
}TextColumn;

typedef enum{
    MENU_TYPE_ADD_MODEL,
    MENU_TYPE_ADD_TEXTURE    
}TextMenuType;

typedef void(*ExecuteFunction)(int);

typedef struct {
    bool show;
    bool element_selected;
    bool execute;
    unsigned int actual_element_select;
    ExecuteFunction execute_function;
    TextMenuType type;
    char text_for_action[30];
}TextMenu;


TextMenu add_element_menu;

 int mark_id;
 float open_file;
bool add_texture;

float pixel_size_x;
float pixel_size_y;

#endif