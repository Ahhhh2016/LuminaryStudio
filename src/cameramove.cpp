#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"

#include "glm/gtx/transform.hpp"



// ================== Project 6: Action!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        float speed = 0.005f;

        cameraData.look = glm::rotate(glm::mat4(1.0f), -deltaX * speed, glm::vec3(0.0f, 1.0f, 0.0f)) * cameraData.look;
        cameraData.look = glm::rotate(glm::mat4(1.0f), -deltaY * speed, glm::normalize(glm::cross(glm::vec3(cameraData.look), glm::vec3(cameraData.up)))) * cameraData.look;
        camera.initialize(cameraData);

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::update_phy_shape(float dt)
{
    for (auto &s : phy_shapes)
    {
        if (s.apply_physics)
        {
            s.f = -s.b * s.v + wind + glm::vec3(0.0f, rand_float(s.lift_force - 0.5f, s.lift_force), 0.0f) + glm::vec3(0.0f, - 9.8f * s.m, 0.0f);
            s.v = (s.f * dt + s.v * s.m) / s.m;
            glm::vec3 ds = s.v * dt;
            for (int j=0; j < s.shape.size(); j++)
            {
                //printf("ds: %f, %f, %f\n", ds[0], ds[1], ds[2]);
                for (int i = 0; i < s.vertexData[j].size() / 8; i++)
                {
                    s.vertexData[j][i * 8 + 0] = s.vertexData[j][i * 8 + 0] + ds[0];
                    s.vertexData[j][i * 8 + 1] = s.vertexData[j][i * 8 + 1] + ds[1];
                    s.vertexData[j][i * 8 + 2] = s.vertexData[j][i * 8 + 2] + ds[2];
                    s.vertexData[j][i * 8 + 3] = s.vertexData[j][i * 8 + 3] + ds[0];
                    s.vertexData[j][i * 8 + 4] = s.vertexData[j][i * 8 + 4] + ds[1];
                    s.vertexData[j][i * 8 + 5] = s.vertexData[j][i * 8 + 5] + ds[2];

                                    }
            }

            s.bottom_center = ds;

        }



    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    if (!stop)
        update_phy_shape(deltaTime);

    m_elapsedTimer.restart();

    float moveSpeed = 0.03f;
    d_time += moveSpeed * deltaTime;
    accumulate_time += deltaTime;
    if(auto_camera) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(deltaTime * 9.0f), glm::vec3(0, 1, 0));
        cameraData.look = rotationMatrix * cameraData.look;
        cameraData.pos += glm::vec4(0, 0.5f, 0, 0) * deltaTime;
        camera.initialize(cameraData);

    }
    if(accumulate_time > 10.f) {
//        std::cout<<"look: "<<cameraData.look[0]<<" "<<cameraData.look[1]<<" "<<cameraData.look[2]<<std::endl;
//        std::cout<<"pos: "<<cameraData.pos[0]<<" "<<cameraData.pos[1]<<" "<<cameraData.pos[2]<<std::endl;
//        std::cout<<"up: "<<cameraData.up[0]<<" "<<cameraData.up[1]<<" "<<cameraData.up[2]<<std::endl;

//        if(skybox_index != 2)
        auto faces = all_skybox[skybox_index];
        skybox_index = (skybox_index + 1) % 4;

        // if (skybox_index == 1)
        //     fire_factor = 1.66f;

        // if (skybox_index == 2)
        //     fire_factor = 1.63f;

        // if (skybox_index == 3)
        //     fire_factor = 1.62f;

        accumulate_time = 0.f;
        glGenTextures(1, &cubemapTexture);
        // binding
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        // load each image and copy into a side of the cube-map texture
        loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, faces[4]);
        loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, faces[5]);
        loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, faces[2]);
        loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, faces[3]);
        loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, faces[1]);
        loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_X, faces[0]);
        // format cube map texture
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // unbinding
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    }
    if (d_time > 1.0f) {
        if(radius_light >= 0.12) radius_light = 0.09;
        else radius_light = 0.12;
    }
    offset = glm::vec2(rand_float(0.0, 0.03), rand_float(0.0, 0.02));


    if (d_time > 1.0f)
        d_time = 0.0f;

    fire_image_num += 1;
    if (fire_image_num > 234)
        fire_image_num = 0;

    glm::vec4 move_dir = glm::vec4(0.0f);

    // Use deltaTime and m_keyMap here to move around
    if (m_keyMap[Qt::Key_W])
        move_dir += cameraData.look;
    if (m_keyMap[Qt::Key_S])
        move_dir += -cameraData.look;
    if (m_keyMap[Qt::Key_A])
        move_dir += glm::vec4(glm::cross(glm::vec3(cameraData.up), glm::vec3(cameraData.look)), 0.0f);
    if (m_keyMap[Qt::Key_D])
        move_dir += glm::vec4(glm::cross(glm::vec3(cameraData.look), glm::vec3(cameraData.up)), 0.0f);
    if (m_keyMap[Qt::Key_Space])
        move_dir += glm::vec4(0, 1.0f, 0, 0);
    if (m_keyMap[Qt::Key_Control])
        move_dir += glm::vec4(0, -1.0f, 0, 0);

    if (move_dir != glm::vec4(0.0f))
    {
        cameraData.pos += glm::normalize(move_dir) * deltaTime * 20.0f;
        camera.initialize(cameraData);

        // std::cout << camera.pos[0] << " " << camera.pos[1] << " " << camera.pos[2] << " " << camera.pos[3] << std::endl;
    }


    update(); // asks for a PaintGL() call to occur
}
