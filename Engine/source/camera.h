//
// Created by pavon on 6/23/19.
//

#ifndef PAVONTHEGAME_CAMERA_H
#define PAVONTHEGAME_CAMERA_H

#include "third_party/cglm/cglm.h"

#include "Engine/components/components.h"

CameraComponent main_camera;
CameraComponent saved_camera;

float camera_heigth_screen;
float camera_width_screen;
versor camera_rotation;

void init_camera();
void update_look_at();

bool move_camera_input;

float camera_rotate_angle;

#endif //PAVONTHEGAME_CAMERA_H