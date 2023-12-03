#include <stdexcept>
#include "camera.h"
#include "settings.h"

#include "glm/gtx/transform.hpp"

Camera::Camera() {}

void Camera::initialize(SceneCameraData cameraData)
{
    height_angle = cameraData.heightAngle;
    look = cameraData.look;
    up = cameraData.up;
    pos = cameraData.pos;
    update(settings.nearPlane, settings.farPlane);
}
void Camera::update(float near_plane, float far_plane)
{
    // view mat
    glm::vec4 u, v, w;
    w = -look / glm::length(look);
    v = (up - glm::dot(up, w) * w) / glm::length(up - glm::dot(up, w) * w);
    u = glm::vec4(glm::cross(glm::vec3(v.x, v.y, v.z), glm::vec3(w.x, w.y, w.z)), 0);

    view_mat =  glm::mat4(u[0], v[0], w[0], 0.0f, u[1], v[1], w[1], 0.0f, u[2], v[2], w[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f) * glm::translate(glm::vec3{-pos[0], -pos[1], -pos[2]});

    // proj mat
    float tan_theta_h = glm::tan(height_angle / 2.0f);
    float tan_theta_w = tan_theta_h * aspect_ratio;
    float c = -near_plane / far_plane;

    glm::mat4 S(1.0f / far_plane / tan_theta_w,0.0f,0.0f,0.0f,  0.0f,1.0f / far_plane / tan_theta_h,0.0f,0.0f,  0.0f,0.0f,1.0f / far_plane,0.0f,  0.0f,0.0f,0.0f,1.0f);
    glm::mat4 M(1.0f,0.0f,0.0f,0.0f,  0.0f,1.0f,0.0f,0.0f,  0.0f,0.0f,1.0f/(1.0f+c),-1.0f,  0.0f,0.0f,-c/(1.0f+c),0.0f);
    glm::mat4 remap(1.0f,0.0f,0.0f,0.0f,  0.0f,1.0f,0.0f,0.0f,  0.0f,0.0f,-2.0f,0.0f, 0.0f,0.0f,-1.0f,1.0f);

    proj_mat = remap * M * S;
}
