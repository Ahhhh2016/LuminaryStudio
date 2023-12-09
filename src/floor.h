#pragma once

#include <GL/glew.h>
#include <iostream>
#include <ctime>
#include <math.h>
#include <glm/glm.hpp>
#include "utils/sceneparser.h"
#include "utils/scenefilereader.h"
#include "utils/scenedata.h"
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
