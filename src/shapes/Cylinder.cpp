#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2, int _repeatU, int _repeatV)
{
    repeatU = _repeatU;
    repeatV = _repeatV;
    m_vertexData = std::vector<float>();
    m_param1 = std::max(1, param1);
    m_param2 = std::max(3, param2);
    setVertexData();
}

glm::vec3 set_y(glm::vec3 vec)
{
    vec.y = 0.0f;
    return vec;
}

void Cylinder::insert_data(glm::vec3 pos, glm::vec3 normal)
{
    insertVec3(m_vertexData, pos);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, calculate_uv(pos));
}

glm::vec2 Cylinder::calculate_uv(glm::vec3 pos)
{
    float u, v;
    float px = pos.x, py = pos.y, pz = pos.z;
    float theta = atan2(pz, px) / 2 / 3.14;
    if (py >= 0.49999f)
    {
        u = px + 0.5f;
        v = 0.5f - pz;
    }
    else if (py <= -0.49999f)
    {
        u = px + 0.5f;
        v = pz + 0.5f;
    }
    else
    {
        if (px == 0)
            u = 0.5f;
        else
        {
            if (theta < 0)
                u = -theta;
            else
                u = 1 - theta;
        }
        v = py + 0.5f;
    }
    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    float u_prime = u * repeatU - floor(u * repeatU);
    float v_prime = v * repeatV - floor(v * repeatV);

    return glm::vec2(u_prime, v_prime);
}

void Cylinder::makeSideFace(float currentTheta, float nextTheta)
{
    float r = 0.5f;

    for (int i = 0; i < m_param1; i++)
    {
        float currentHeight = float(i) / m_param1 - 1.0f / 2.0f;
        float nextHeight = float(i + 1) / m_param1 - 1.0f / 2.0f;

        glm::vec3 topLeft    (r * glm::sin(currentTheta), nextHeight, r * glm::cos(currentTheta));
        glm::vec3 topRight   (r * glm::sin(nextTheta)   , nextHeight, r * glm::cos(nextTheta)   );
        glm::vec3 bottomRight(r * glm::sin(nextTheta)   , currentHeight, r * glm::cos(nextTheta)   );
        glm::vec3 bottomLeft (r * glm::sin(currentTheta), currentHeight, r  * glm::cos(currentTheta));

        insert_data(topLeft, glm::normalize(set_y(topLeft)));
        insert_data(bottomLeft, glm::normalize(set_y(bottomLeft)));
        insert_data(topRight, glm::normalize(set_y(topRight)));
        // // // // // // // // // // //
        insert_data(bottomRight, glm::normalize(set_y(bottomRight)));
        insert_data(topRight, glm::normalize(set_y(topRight)));
        insert_data(bottomLeft, glm::normalize(set_y(bottomLeft)));


    }
}

void Cylinder::makeCircleFace(float currentTheta, float nextTheta, bool isTop)
{
    float height = isTop ? 0.5f : -0.5f;
    glm::vec3 normal = isTop ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, -1.0f, 0.0f);

    for (int i = 0; i < m_param1; i++)
    {
        float currentR = float(i) / m_param1 * 1.0f / 2.0f;
        float nextR = float(i + 1) / m_param1 * 1.0f / 2.0f;

        glm::vec3 topLeft    (currentR * glm::sin(currentTheta), height, currentR * glm::cos(currentTheta));
        glm::vec3 topRight   (currentR * glm::sin(nextTheta)   , height, currentR * glm::cos(nextTheta)   );
        glm::vec3 bottomRight(nextR * glm::sin(nextTheta)   , height, nextR * glm::cos(nextTheta)   );
        glm::vec3 bottomLeft (nextR * glm::sin(currentTheta), height, nextR * glm::cos(currentTheta));

        if (!isTop)
        {
            std::swap(topLeft, topRight);
            std::swap(bottomLeft, bottomRight);
        }

        insert_data(topLeft, normal);
        insert_data(bottomLeft, normal);
        insert_data(topRight, normal);
        // // // // // // // // // // //
        insert_data(bottomRight, normal);
        insert_data(topRight, normal);
        insert_data(bottomLeft, normal);
    }
}

void Cylinder::setVertexData()
{
    float thetaStep = glm::radians(360.f / m_param2);
    for (int i = 0; i < m_param2; i++)
    {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeSideFace(currentTheta, nextTheta);

        makeCircleFace(currentTheta, nextTheta, true);
        makeCircleFace(currentTheta, nextTheta, false);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::insertVec2(std::vector<float> &data, glm::vec2 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
}
