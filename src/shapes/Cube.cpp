#include "Cube.h"

void Cube::updateParams(int param1, int _repeatU, int _repeatV)
{
    repeatU = _repeatU;
    repeatV = _repeatV;
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::insert_data(glm::vec3 pos, glm::vec3 normal)
{
    insertVec3(m_vertexData, pos);
    insertVec3(m_vertexData, normal);
    insertVec2(m_vertexData, calculate_uv(pos));
}

glm::vec2 Cube::calculate_uv(glm::vec3 pos)
{
    float u, v;
    float px = pos.x, py = pos.y, pz = pos.z;
    if (px >= 0.499f)
    {
        u = 0.5f - pz;
        v = py + 0.5f;
    }
    else if (px <= -0.49999f)
    {
        u = pz + 0.5f;
        v = py + 0.5f;
    }
    else if (py >= 0.49999f)
    {
        u = px + 0.5f;
        v = 0.5f - pz;
    }
    else if (py <= -0.49999f)
    {
        u = px + 0.5f;
        v = pz + 0.5f;
    }
    else if (pz >= 0.49999f)
    {
        u = px + 0.5f;
        v = py + 0.5f;
    }
    else if (pz <= -0.49999f)
    {
        u = 0.5f - px;
        v = py + 0.5f;
    }

    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    // repeatU = 4;
    // repeatV = 4;

    float u_prime = u * repeatU - floor(u * repeatU);
    float v_prime = v * repeatV - floor(v * repeatV);

    return glm::vec2(u_prime, v_prime);
}

void Cube::makeTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight)
{
    glm::vec3 normal = glm::normalize(glm::cross(bottomLeft - topLeft, bottomRight - topLeft));

    insert_data(topLeft, normal);
    insert_data(bottomLeft, normal);
    insert_data(bottomRight, normal);
    // // // // // // // // // // //
    insert_data(bottomRight, normal);
    insert_data(topRight, normal);
    insert_data(topLeft, normal);
}

void Cube::makeFace(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight)
{
    glm::vec3 dx = (topRight - topLeft) / float(m_param1);
    glm::vec3 dy = (topRight - bottomRight) / float(m_param1);
    for (int row = 0; row < m_param1; row++)
    {
        for (int col = 0; col < m_param1; col++)
        {
            makeTile(bottomLeft + float(row + 1) * dy + float(col) * dx, bottomLeft + float(row + 1) * dy + float(col + 1) * dx, bottomLeft + float(row) * dy + float(col) * dx, bottomLeft + float(row) * dy + float(col + 1) * dx);
        }
    }
}

void Cube::setVertexData()
{
     makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
              glm::vec3( 0.5f,  0.5f, 0.5f),
              glm::vec3(-0.5f, -0.5f, 0.5f),
              glm::vec3( 0.5f, -0.5f, 0.5f));
     makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3( 0.5f, -0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f));
     makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
              glm::vec3(-0.5f, -0.5f,  0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f));
     makeFace(glm::vec3( 0.5f, -0.5f,  0.5f),
              glm::vec3( 0.5f,  0.5f,  0.5f),
              glm::vec3( 0.5f, -0.5f, -0.5f),
              glm::vec3( 0.5f,  0.5f, -0.5f));
     makeFace(glm::vec3( 0.5f,  0.5f, 0.5f),
              glm::vec3(-0.5f,  0.5f, 0.5f),
              glm::vec3(0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f));
     makeFace(glm::vec3(-0.5f,  -0.5f,  0.5f),
              glm::vec3( 0.5f,  -0.5f,  0.5f),
              glm::vec3(-0.5f,  -0.5f, -0.5f),
              glm::vec3( 0.5f,  -0.5f, -0.5f));
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cube::insertVec2(std::vector<float> &data, glm::vec2 v)
{
    data.push_back(v.x);
    data.push_back(v.y);
}
