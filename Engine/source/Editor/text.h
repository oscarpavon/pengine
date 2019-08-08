#ifndef EDITOR_TEXT_H
#define EDITOR_TEXT_H


#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>

#include <stdbool.h>
#include "../Engine/array.h"

#include "input.h"

#define DIRECTORY_MODELS 0
#define DIRECTORY_TEXTURES 1


void init_text_renderer();
void render_text(const char *text, float x, float y, float sx, float sy , bool mark);
void draw_directory_files();
void draw_directory_file_type(unsigned short int type);

void text_renderer_loop();

void update_text_renderer_window_size();

FT_Face face;

static inline void set_text_size(float size){
    FT_Set_Pixel_Sizes(face, 0, size);  
}

typedef enum{
    MENU_TYPE_ADD_MODEL,
    MENU_TYPE_ADD_TEXTURE    
}TextMenuType;

typedef struct TextMenu TextMenu;

typedef void(*TextMenuFunction)(TextMenu*);

typedef struct TextMenu{
    char name[30];
    bool show;
    bool element_selected;
    bool execute;
    unsigned int actual_element_select;
    TextMenuFunction execute_function;
    TextMenuFunction draw_text_funtion;
    TextMenuType type;
    unsigned short int element_count;
    unsigned short int text_size;
    char text_for_action[30];
    Key* open_key;
    int mods_key;
}TextMenu;


TextMenu add_element_menu;
TextMenu menu_add_texture;
TextMenu menu_editor_element_list;

TextMenu menu_add_native_editor_element;

TextMenu menu_show_gui_elements;

int mark_id;

Array menus;

#endif