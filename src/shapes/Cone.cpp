#include "Cone.h"

void Cone::updateParams(int param1, int param2, int _repeatU, int _repeatV)
{
    repeatU = _repeatU;
    repeatV = _repeatV;
    m_vertexData = std::vector<float>();
    m_param1 = std::max(1, param1);
    m_param2 = std::max(3, param2);
    setVertexData();

}

void Cone::insert_data(glm::vec3 pos, glm::vec3 normal)
{
    insertVec3(m_vertexData, pos);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, calculate_uv(pos));
}

glm::vec2 Cone::calculate_uv(glm::vec3 pos)
{
    float u, v;
    float px = pos.x, py = pos.y, pz = pos.z;
    float theta = atan2(pz, px) / 2 / 3.14;
    v = py + 0.5f;
    if (py <= -0.49999f)
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
    }
    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    float u_prime = u * repeatU - floor(u * repeatU);
    float v_prime = v * repeatV - floor(v * repeatV);

    return glm::vec2(u_prime, v_prime);
}

glm::vec3 get_normal(glm::vec3 vec)
{
    return glm::normalize(glm::vec3(2*vec.x, -0.5f * vec.y + 0.25f, 2*vec.z));
}

void Cone::makeSideFace(float currentTheta, float nextTheta)
{
    float r = 0.5f;

    for (int i = 0; i < m_param1; i++)
    {
        float currentRatio = 1.0f - float(i) / m_param1;
        float nextRatio = 1.0f - float(i + 1) / m_param1;

        float currentHeight = float(i) / m_param1 - 1.0f / 2.0f;
        float nextHeight = float(i + 1) / m_param1 - 1.0f / 2.0f;

        glm::vec3 topLeft    (r * glm::sin(currentTheta) * nextRatio, nextHeight, r * glm::cos(currentTheta) * nextRatio);
        glm::vec3 topRight   (r * glm::sin(nextTheta) * nextRatio, nextHeight, r * glm::cos(nextTheta) * nextRatio);
        glm::vec3 bottomRight(r * glm::sin(nextTheta) * currentRatio, currentHeight, r * glm::cos(nextTheta) * currentRatio);
        glm::vec3 bottomLeft (r * glm::sin(currentTheta) * currentRatio, currentHeight, r  * glm::cos(currentTheta) * currentRatio);

        glm::vec3 normal = get_normal(glm::vec3(r * glm::sin((currentTheta + nextTheta) / 2) * currentRatio, currentHeight, r * glm::cos((currentTheta + nextTheta) / 2) * currentRatio));

        bool isTop = i == m_param1 - 1;

        insert_data(topLeft, isTop ? normal : get_normal(topLeft));
        insert_data(bottomLeft, get_normal(bottomLeft));
        insert_data(bottomRight, get_normal(bottomRight));
        // // // // // // // // // // //
        insert_data(bottomRight, get_normal(bottomRight));
        insert_data(topRight, isTop ? normal : get_normal(topRight));
        insert_data(topLeft, isTop ? normal : get_normal(topLeft));
    }
}

void Cone::makeCircleFace(float currentTheta, float nextTheta)
{

    float height = -0.5f;
    glm::vec3 normal = glm::vec3(0.0f, -1.0f, 0.0f);

    for (int i = 0; i < m_param1; i++)
    {
        float currentR = float(i) / m_param1 * 1.0f / 2.0f;
        float nextR = float(i + 1) / m_param1 * 1.0f / 2.0f;

        glm::vec3 topRight    (currentR * glm::sin(currentTheta), height, currentR * glm::cos(currentTheta));
        glm::vec3 topLeft   (currentR * glm::sin(nextTheta)   , height, currentR * glm::cos(nextTheta)   );
        glm::vec3 bottomLeft(nextR * glm::sin(nextTheta)   , height, nextR * glm::cos(nextTheta)   );
        glm::vec3 bottomRight (nextR * glm::sin(currentTheta), height, nextR * glm::cos(currentTheta));

        insert_data(topLeft, normal);
        insert_data(bottomLeft, normal);
        insert_data(bottomRight, normal);
        // // // // // // // // // // //
        insert_data(bottomRight, normal);
        insert_data(topRight, normal);
        insert_data(topLeft, normal);
    }

}

void Cone::setVertexData() {
    // TODO for Project 5: Lights, Camera
    float thetaStep = glm::radians(360.f / m_param2);
    for (int i = 0; i < m_param2; i++)
    {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeSideFace(currentTheta, nextTheta);

        makeCircleFace(currentTheta, nextTheta);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cone::insertVec2(std::vector<float> &data, glm::vec2 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
}
