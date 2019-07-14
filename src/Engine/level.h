/*Created by pavon on 2019/07/10 */
#ifndef ENGINE_LEVEL_H
#define ENGINE_LEVEL_H
#include "../vector.h"

void load_level_to_elements_array(const char* name, Array* load_elements);
void add_loaded_elements(Array* load_elements, ModelArray* editor_models, Array* editor_elements);

#endif //ENGINE_LEVEL_H