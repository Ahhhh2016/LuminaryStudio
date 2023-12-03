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

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

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
        cameraData.pos += glm::normalize(move_dir) * deltaTime * 5.0f;
        camera.initialize(cameraData);
    }

    update(); // asks for a PaintGL() call to occur
}
