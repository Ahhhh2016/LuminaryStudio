#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"
#include <iostream>
#include <random>
#include "glm/gtx/transform.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>


// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent), topCamera(), bottomCamera(), leftCamera(), rightCamera(), frontCamera(), backCamera(), camera()
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);
    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteProgram(m_phong_shader);

    // project 6
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteTextures(1, &m_shape_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteProgram(m_postprocess_shader);
    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);
    glDeleteBuffers(1, &m_defaultFBO);

    glDeleteProgram(m_phong_shader);
    glDeleteProgram(m_skybox_shader);
    glDeleteFramebuffers(1, &m_fbo_texture);

    //texture
    glDeleteTextures(1, &m_shape_texture);

    // dynamic environment mapping
    glDeleteFramebuffers(1, &fbo_cube);
    glDeleteTextures(1, &fbo_tex_cube);
    glDeleteRenderbuffers(1, &fbo_rb_cube);

    // normal Map
    glDeleteVertexArrays(1, &m_vao_normal);
    glDeleteTextures(1, &m_normal_texture);

    // skybox
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &skyboxVAO);

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    m_phong_shader = ShaderLoader::createShaderProgram(":/resources/shaders/phong.vert", ":/resources/shaders/phong.frag");
    m_postprocess_shader = ShaderLoader::createShaderProgram(":/resources/shaders/postprocess.vert", ":/resources/shaders/postprocess.frag");

    //==============project 6================
    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    glUseProgram(m_phong_shader);
    glUniform1i(glGetUniformLocation(m_phong_shader, "skybox"), 0);
    glUniform1i(glGetUniformLocation(m_phong_shader, "dudvMap"), 2);
    glUniform1i(glGetUniformLocation(m_phong_shader, "normalMap"), 3);
    glUseProgram(0);

    // //ini_skybox();
    // for (float& i : skyboxVertices) {
    //     i *= 100;
    // }

    // skybox VAO
    // skybox
    glEnable(GL_DEPTH_TEST);
    m_skybox_shader = ShaderLoader::createShaderProgram(":/resources/shaders/skybox.vert", ":/resources/shaders/skybox.frag");

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size()*sizeof(GLfloat), skyboxVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // load textures
    // -------------

    auto faces = all_skybox[skybox_index];


    //cubemapTexture = loadCubemap(faces);
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

    // init dynamic cubemap
    // depth buffer
    glGenRenderbuffers(1, &fbo_rb_cube);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo_rb_cube);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, skybox_width, skybox_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // texture
    // create the cubemap
    glGenTextures(1, &fbo_tex_cube);
    glActiveTexture(GL_TEXTURE1); // texture slot 1
    glBindTexture(GL_TEXTURE_CUBE_MAP, fbo_tex_cube);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skybox_width, skybox_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenFramebuffers(1, &fbo_cube);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_cube);
    // attach buffer & tex to fbo
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_rb_cube);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_tex_cube, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

    topCamera.initialize(SceneCameraData{   glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),  glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});
    bottomCamera.initialize(SceneCameraData{glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, -1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, -1.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});
    leftCamera.initialize(SceneCameraData{  glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, -1.0f, 0.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});
    rightCamera.initialize(SceneCameraData{ glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),  glm::vec4(0.0f, -1.0f, 0.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});
    frontCamera.initialize(SceneCameraData{ glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, -1.0f, 0.0f), glm::vec4(0.0f, -1.0f, 0.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});
    backCamera.initialize(SceneCameraData{  glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),  glm::vec4(0.0f, -1.0f, 0.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});

    // clipping plane
    //glEnable(GL_CLIP_DISTANCE0);
}

void Realtime::drawFboSide(Camera c)
{
    glm::vec4 pos1 = camera.pos;
    if (phy_shapes.size() > 5 && phy_shapes[5].shape[0].primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
        c.initialize(SceneCameraData{   glm::vec4(0.0f, 0.1f, 0.0f, 0.0f), c.look,  c.up, 3.1f / 2.0f, 0.0f, 0.0f});//glm::vec4(pos1[0], -pos1[1], pos1[2], pos1[3]), c.look,  c.up, 3.1f / 2.0f, 0.0f, 0.0f});
    else
        c.initialize(SceneCameraData{   glm::vec4(pos1[0], -pos1[1], pos1[2], pos1[3]), c.look,  c.up, 3.1f / 2.0f, 0.0f, 0.0f});
    c.update(0.1f, 1000.0f);
    glClearColor(0.1f, 1.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    bool paint_all = false;
    //--------------------------------------------------------------------
    paint_shapes(false, c);
    //---------------------------------------------------------------------
    glBindVertexArray(0);

    // CUBEMAP ---------------------------------------------------------
    glDepthFunc(GL_LEQUAL);
    glUseProgram(m_skybox_shader);

    glm::mat4 view = glm::mat4(glm::mat3(c.view_mat));
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "projection"), 1, GL_FALSE, &c.proj_mat[0][0]);
    glUniform1i(glGetUniformLocation(m_skybox_shader, "skybox"), 0);
    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glUseProgram(0);
    glDepthFunc(GL_LESS); // set depth function back to default
    // CUBEMAP ---------------------------------------------------------
}

void Realtime::paintGL() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_cube);
    glViewport(0, 0, skybox_width, skybox_height);
    for (int i = 0; i < 6; i++)
    {
        switch (i)
        {
        case 0:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fbo_tex_cube, 0);
            drawFboSide(rightCamera);
            break;
        case 1:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fbo_tex_cube, 0);
            drawFboSide(leftCamera);
            break;
        case 2:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fbo_tex_cube, 0);
            drawFboSide(topCamera);
            break;
        case 3:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fbo_tex_cube, 0);
            drawFboSide(bottomCamera);
            break;
        case 4:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fbo_tex_cube, 0);
            drawFboSide(backCamera);
            break;
        case 5:
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, fbo_tex_cube, 0);
            drawFboSide(frontCamera);
            break;
        default:
            std::cerr << "side does not exist" << std::endl;
            break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width * this->devicePixelRatio(), m_screen_height * this->devicePixelRatio());

    glClearColor(0.1f, 1.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, fbo_tex_cube);

    paint_shapes(true, camera);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);

    // CUBEMAP ---------------------------------------------------------
    glDepthFunc(GL_LEQUAL);
    glUseProgram(m_skybox_shader);
    glm::mat4 view1 = glm::mat4(glm::mat3(camera.view_mat));
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "view"), 1, GL_FALSE, &view1[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "projection"), 1, GL_FALSE, &camera.proj_mat[0][0]);
    glUniform1i(glGetUniformLocation(m_skybox_shader, "skybox"), 1);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glDepthFunc(GL_LESS); // set depth function back to default
    // CUBEMAP ---------------------------------------------------------
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    camera.aspect_ratio = float(w) / float(h);
    camera.update(settings.nearPlane, settings.farPlane);

    topCamera.aspect_ratio = 1.0f;
    bottomCamera.aspect_ratio = 1.0f;
    leftCamera.aspect_ratio = 1.0f;
    rightCamera.aspect_ratio = 1.0f;
    frontCamera.aspect_ratio = 1.0f;
    backCamera.aspect_ratio = 1.0f;

    //================project 6================
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Task 34: Regenerate your FBOs
    // glDeleteTextures(1, &m_fbo_texture);
    // glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    // glDeleteFramebuffers(1, &m_fbo);

    //makeFBO();
    //================project 6================
}

void Realtime::sceneChanged()
{
    RenderData metaData;
    bool success = SceneParser::parse(settings.sceneFilePath, metaData);
    if (!success) {
        std::cerr << "Error loading scene: \"" << settings.sceneFilePath << "\"" << std::endl;
    }
    shapes = metaData.shapes;
    lights = metaData.lights;
    globalData = metaData.globalData;
    cameraData = metaData.cameraData;
    camera.initialize(cameraData);

    //camera.initialize(SceneCameraData{   glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 10.0f, 0.0f, 0.0f),  glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});

    //update shapes
    settings.shapeParameter1 = 10;
    settings.shapeParameter2 = 10;
    parameter1 = settings.shapeParameter1;
    parameter2 = settings.shapeParameter2;
    use_texture = settings.use_texture;
    adaptive_detail = settings.adaptive_detail;
    cone.updateParams(parameter1, parameter2, 1.0f, 1.0f);
    cube.updateParams(parameter1, 1.0f, 1.0f);
    cylinder.updateParams(parameter1, parameter2, 1.0f, 1.0f);
    sphere.updateParams(parameter1, parameter2, 1.0f, 1.0f);

    settings.nearPlane = 0.1f;
    settings.farPlane = 1000.0f;

    //generate phy shapes
    ini_phy_shapes();


    init_shapes();

    update(); // asks for a PaintGL() call to occur
}

std::vector<float> generateMeshVertexData(const std::string& filePath) {
    std::string line;
    std::ifstream objFile(filePath); // Adjust the path as needed

    std::vector<float> vertices, uvs, normals, vertexData;

    if (objFile.is_open()) {
        while (getline(objFile, line)) {
            std::stringstream ss(line);
            std::string lineType;
            ss >> lineType;

            if (lineType == "v") {
                float x, y, z;
                ss >> x >> y >> z;
                vertices.insert(vertices.end(), {x, y, z});
            }
            else if (lineType == "vt") {
                float u, v;
                ss >> u >> v;
                uvs.insert(uvs.end(), {u, v});
            }
            else if (lineType == "vn") {
                float nx, ny, nz;
                ss >> nx >> ny >> nz;
                normals.insert(normals.end(), {nx, ny, nz});
            }
            else if (lineType == "f") {
                int vIdx[4], uvIdx[4], nIdx[4];
                char slash;

                // Assuming the format is v/vt/vn
                for (int i = 0; i < 4; ++i) {
                    ss >> vIdx[i] >> slash >> uvIdx[i] >> slash >> nIdx[i];
                    vIdx[i]--; uvIdx[i]--; nIdx[i]--;  // Adjust for 0-based indexing
                }

                if (vIdx[3] == -1)
                {
                    // Convert quad to two triangles
                    int quadToTri[3] = {0, 1, 2}; // Indices to form two triangles from a quad
                    for (int i = 0; i < 3; ++i) {
                        int idx = quadToTri[i];
                        vertexData.insert(vertexData.end(), {vertices[vIdx[idx] * 3], vertices[vIdx[idx] * 3 + 1], vertices[vIdx[idx] * 3 + 2]});
                        vertexData.insert(vertexData.end(), {normals[nIdx[idx] * 3], normals[nIdx[idx] * 3 + 1], normals[nIdx[idx] * 3 + 2]});
                        vertexData.insert(vertexData.end(), {uvs[uvIdx[idx] * 2], uvs[uvIdx[idx] * 2 + 1]});
                    }
                }
                else
                {
                    // Convert quad to two triangles
                    int quadToTri[6] = {0, 1, 2, 0, 2, 3}; // Indices to form two triangles from a quad
                    for (int i = 0; i < 6; ++i) {
                        int idx = quadToTri[i];
                        vertexData.insert(vertexData.end(), {vertices[vIdx[idx] * 3], vertices[vIdx[idx] * 3 + 1], vertices[vIdx[idx] * 3 + 2]});
                        vertexData.insert(vertexData.end(), {normals[nIdx[idx] * 3], normals[nIdx[idx] * 3 + 1], normals[nIdx[idx] * 3 + 2]});
                        vertexData.insert(vertexData.end(), {uvs[uvIdx[idx] * 2], uvs[uvIdx[idx] * 2 + 1]});
                    }
                }

            }
        }
        objFile.close();
    }

    return vertexData;
}

std::vector<std::vector<float>> Realtime::generate_vertex_data(RenderShapeData s)
{
    settings.shapeParameter1 = 10;
    settings.shapeParameter2 = 10;
    std::vector<std::vector<float>> temp;
    if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE)
    {
        cube.updateParams(settings.shapeParameter1, 1.0f, 1.0f);
        temp.push_back(cube.generateShape());
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_CONE)
    {
        cone.updateParams(settings.shapeParameter1, settings.shapeParameter2, 1.0f, 1.0f);
        temp.push_back(cone.generateShape());
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
    {
        sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2, 1.0f, 1.0f);
        temp.push_back(sphere.generateShape());
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
    {
        cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2, 1.0f, 1.0f);
        temp.push_back(cylinder.generateShape());
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_MESH)
    {
        temp.push_back(generateMeshVertexData(s.primitive.meshfile + "_main"));
        temp.push_back(generateMeshVertexData(s.primitive.meshfile + "_bottom"));
    }

    for (int j=0; j<temp.size(); j++)
    {
        for (int i = 0; i < temp[j].size() / 8; i++)
        {
            glm::vec3 world_pos = glm::vec3(s.ctm * glm::vec4(temp[j][i * 8], temp[j][i * 8 + 1], temp[j][i * 8 + 2], 1.0f));
            temp[j][i * 8] = world_pos[0];
            temp[j][i * 8 + 1] = world_pos[1];
            temp[j][i * 8 + 2] = world_pos[2];
            glm::vec3 world_normal = glm::normalize(glm::inverse(glm::transpose(glm::mat3(s.ctm))) * normalize(glm::vec3(temp[j][i * 8 + 3], temp[j][i * 8 + 4], temp[j][i * 8 + 5])));
            temp[j][i * 8 + 3] = world_normal[0];
            temp[j][i * 8 + 4] = world_normal[1];
            temp[j][i * 8 + 5] = world_normal[2];
        }
    }


    return temp;
}

float Realtime::rand_float(float min_float, float max_float)
{
    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 gen(rd()); // Seed the generator

    std::uniform_real_distribution<> dis(min_float, max_float);

    return dis(gen);
}

std::vector<std::vector<float>> Realtime::generate_random_vertex_data(std::vector<std::vector<float>> vbos)
{
    float rndx = rand_float(-150.0f, 150.0f);
    float rndy = rand_float(-100.0f, 0.0f);
    float rndz = rand_float(-150.0f, 150.0f);
    std::vector<float> res;
    for (std::vector<float>& vbo : vbos)
    {
        for (int i = 0; i < vbo.size() / 8; i++)
        {
            res.push_back(vbo[i * 8 + 0] + rndx);
            res.push_back(vbo[i * 8 + 1] + rndy);
            res.push_back(vbo[i * 8 + 2] + rndz);
            res.push_back(vbo[i * 8 + 3]);
            res.push_back(vbo[i * 8 + 4]);
            res.push_back(vbo[i * 8 + 5]);
            res.push_back(vbo[i * 8 + 6]);
            res.push_back(vbo[i * 8 + 7]);
        }
    }
    return std::vector<std::vector<float>> {res};
}


void Realtime::ini_phy_shapes()
{
    phy_shapes.clear();
    int index = 0;
    for (auto &s : shapes)
    {
        if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE)//(index == 5)
        {
            phy_shapes.push_back(physics_shape{false, true, true, 0.0f, 0.0f, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, generate_vertex_data(s), std::vector<RenderShapeData>{s}});
        }
        else if (s.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) //(index == 6)
        {
            phy_shapes.push_back(physics_shape{false, true, false, 0.0f, 0.0f, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, generate_vertex_data(s), std::vector<RenderShapeData>{s}});
        }
        else if (s.primitive.type == PrimitiveType::PRIMITIVE_MESH)
        {
            //phy_shapes.push_back(physics_shape{open_physics, false, false, 0.35f, 5.5f, glm::vec3(0.0f), glm::vec3(0.0f), rand_float(0.4f, 0.5f), generate_vertex_data(s), std::vector<RenderShapeData>{s}});
            for (int i = 0; i < new_rand_num; i++)
            {
                glm::mat4 temp_ctm = s.ctm * glm::translate(glm::vec3(0.0f, 16.5f, 0.0f)) * glm::scale(glm::vec3(2.0f, 0.1f, 2.0f));
                RenderShapeData temp_cone{ScenePrimitive{PrimitiveType::PRIMITIVE_CYLINDER, s.primitive.material, ""}, temp_ctm, glm::inverse(temp_ctm)};
                auto ini_vertex_data = generate_vertex_data(s);
                phy_shapes.push_back(physics_shape{open_physics, false, false, 0.35f, 5.5f, glm::vec3(0.0f), glm::vec3(0.0f), rand_float(0.4f, 0.5f), generate_random_vertex_data(ini_vertex_data), std::vector<RenderShapeData>{s}});
            }
        }
        else
        {
            phy_shapes.push_back(physics_shape{false, false, false, 0.0f, 0.0f, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, generate_vertex_data(s), std::vector<RenderShapeData>{s}});
        }
        index++;
    }
}

void Realtime::settingsChanged()
{
    if (settings.nearPlane != near_plane || settings.farPlane != far_plane)
    {
        near_plane = 0.1f;
        far_plane = 1000.0f;
        camera.update(near_plane, far_plane);
    }

    if (settings.shapeParameter1 != parameter1 || settings.shapeParameter2 != parameter2 || (settings.adaptive_detail != adaptive_detail && !settings.adaptive_detail))
    {
        parameter1 = 10;
        parameter2 = 10;
        cone.updateParams(parameter1, parameter2, 1.0f, 1.0f);
        cube.updateParams(parameter1, 1.0f, 1.0f);
        cylinder.updateParams(parameter1, parameter2, 1.0f, 1.0f);
        sphere.updateParams(parameter1, parameter2, 1.0f, 1.0f);

        //adaptive_detail = settings.adaptive_detail;
    }


    if (settings.sharpen_filter != sharpen_filter)
    {
        sharpen_filter = settings.sharpen_filter;
    }

    if (settings.invert_filter != invert_filter)
    {
        invert_filter = settings.invert_filter;
    }

    if (settings.grayscale_filter != grayscale_filter)
    {
        grayscale_filter = settings.grayscale_filter;
    }

    if (settings.blur_filter != blur_filter)
    {
        //blur_filter = settings.blur_filter;
        wind = glm::vec3(0.3f, 0.0f, 0.3f);
    }

    if (settings.use_texture != use_texture)
    {
        //use_texture = settings.use_texture;
        stop = !stop;
    }

    update(); // asks for a PaintGL() call to occur
}

