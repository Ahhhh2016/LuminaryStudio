#include "Sphere.h"

void Sphere::updateParams(int param1, int param2, int _repeatU, int _repeatV)
{
    repeatU = _repeatU;
    repeatV = _repeatV;
    m_vertexData = std::vector<float>();
    m_param1 = std::max(2, param1);
    m_param2 = std::max(3, param2);
    setVertexData();
}

void Sphere::insert_data(glm::vec3 vec)
{
    glm::vec3 center(0.0f, 0.0f, 0.0f);

    insertVec3(m_vertexData, vec);
    insertVec3(m_vertexData, glm::normalize(vec - center));
    insertVec2(m_vertexData, calculate_uv(vec));
}

glm::vec2 Sphere::calculate_uv(glm::vec3 pos)
{
    float u, v;
    float px = pos.x, py = pos.y, pz = pos.z;
    float theta = atan2(pz, px) / 2 / 3.14;
    if (px == 0)
        u = 0.5f;
    else
    {
        if (theta < 0)
            u = -theta;
        else
            u = 1 - theta;
    }
    v = asin(py / 0.5f) / 3.14 + 0.5f;

    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    float u_prime = u * repeatU - floor(u * repeatU);
    float v_prime = v * repeatV - floor(v * repeatV);

    return glm::vec2(u_prime, v_prime);
}

void Sphere::makeTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight)
{
    insert_data(topLeft);
    insert_data(bottomLeft);
    insert_data(bottomRight);
    // // // // // // // // // // //
    insert_data(bottomRight);
    insert_data(topRight);
    insert_data(topLeft);
}

void Sphere::makeWedge(float currentTheta, float nextTheta)
{
    float r = 0.5f;

    for (int i = 0; i < m_param1; i++)
    {
        float phi2 = glm::radians(float(i) * 180.0f / float(m_param1));
        float phi1 = glm::radians(float(i + 1) * 180.0f / float(m_param1));

        glm::vec3 topLeft    (r * glm::sin(phi2) * glm::sin(currentTheta), r * glm::cos(phi2), r * glm::sin(phi2) * glm::cos(currentTheta));
        glm::vec3 topRight   (r * glm::sin(phi2) * glm::sin(nextTheta)   , r * glm::cos(phi2), r * glm::sin(phi2) * glm::cos(nextTheta)   );
        glm::vec3 bottomRight(r * glm::sin(phi1) * glm::sin(nextTheta)   , r * glm::cos(phi1), r * glm::sin(phi1) * glm::cos(nextTheta)   );
        glm::vec3 bottomLeft (r * glm::sin(phi1) * glm::sin(currentTheta), r * glm::cos(phi1), r * glm::sin(phi1) * glm::cos(currentTheta));

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::setVertexData()
{
    float thetaStep = glm::radians(360.f / m_param2);
    for (int i = 0; i < m_param2; i++)
    {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Sphere::insertVec2(std::vector<float> &data, glm::vec2 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
}
