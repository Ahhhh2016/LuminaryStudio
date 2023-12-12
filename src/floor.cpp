#include "floor.h"

Floor::Floor() {
}

void Floor::initialize() {
    float width = 50.0f;
    float vertices[] = {
        width,0,  width,
        width,0, -width,
        -width,0, -width,

        width,0,  width,
        -width,0, -width,
        -width,0,  width
    };
    m_floor_shader = ShaderLoader::createShaderProgram(":/resources/shaders/floor.vert", ":/resources/shaders/floor.frag");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Floor::~Floor() {
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Floor::render(Camera c) {
    // printf("yes");

    glUseProgram(m_floor_shader);
//	shader->use();
//	shader->setMat4("model", model);
//	shader->setMat4("view", view);
//	shader->setMat4("projection", projection);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(m_floor_shader, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_floor_shader, "view"), 1, GL_FALSE, &c.view_mat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_floor_shader, "projection"), 1, GL_FALSE, &c.proj_mat[0][0]);



	glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
    glUseProgram(0);
}
