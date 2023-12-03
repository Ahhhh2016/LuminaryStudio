#pragma once

#include "utils/scenedata.h"
#include <glm/glm.hpp>

class Camera {
public:
    Camera();

    void initialize(SceneCameraData cameraData);
    void update(float near_plane, float far_plane);

    glm::vec4 pos, look, up;
    float height_angle;
    glm::mat4 view_mat;
    float aspect_ratio;
    glm::mat4 proj_mat;
};
