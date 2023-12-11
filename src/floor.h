#ifndef FLOOR_H
#define FLOOR_H
#pragma once

#include <GL/glew.h>
#include <iostream>
#include <ctime>
#include <math.h>
#include <glm/glm.hpp>
#include "camera/camera.h"
#include "utils/shaderloader.h"

class Floor {
private:

	GLuint VBO, VAO;
    GLuint m_floor_shader;

public:
	Floor();

	~Floor();

    void initialize();

    void render(Camera c);

};


#endif // FLOOR_H
