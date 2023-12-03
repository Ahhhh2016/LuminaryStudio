#pragma once

#include <vector>
#include <glm/glm.hpp>

class Cylinder
{
public:
    void updateParams(int param1, int param2, int _repeatU, int _repeatV);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void insertVec2(std::vector<float> &data, glm::vec2 v);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void insert_data(glm::vec3 pos, glm::vec3 normal);
    void makeCircleFace(float currentTheta, float nextTheta, bool isTop);
    void makeSideFace(float currentTheta, float nextTheta);
    glm::vec2 calculate_uv(glm::vec3 pos);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
    int repeatU, repeatV;
};
