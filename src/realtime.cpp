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
    glUseProgram(0);

    //ini_skybox();
    for (float& i : skyboxVertices) {
        i *= 100;
    }

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
    std::vector<std::string> faces
        {
        "./resources/skybox/right.jpg",
        "./resources/skybox/left.jpg",

            "./resources/skybox/top.jpg",
            "./resources/skybox/bottom.jpg",
        "./resources/skybox/back.jpg",
         "./resources/skybox/front.jpg",
        };

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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 2048, 2048);
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
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
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
    c.initialize(SceneCameraData{   glm::vec4(pos1[0], 10.0f - pos1[1], pos1[2], pos1[3]), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),  glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), 3.1f / 2.0f, 0.0f, 0.0f});
    c.update(0.1f, 1000.0f);
    glClearColor(0.1f, 1.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    //--------------------------------------------------------------------
    glUseProgram(m_phong_shader);

    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "view_matrix"), 1, GL_FALSE, &c.view_mat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "projection_matrix"), 1, GL_FALSE, &c.proj_mat[0][0]);
    glUniform1f(glGetUniformLocation(m_phong_shader, "ka"), globalData.ka);
    glUniform1f(glGetUniformLocation(m_phong_shader, "kd"), globalData.kd);
    glUniform1f(glGetUniformLocation(m_phong_shader, "ks"), globalData.ks);
    glUniform1i(glGetUniformLocation(m_phong_shader, "num_lights"), lights.size());
    for (int i = 0; i < lights.size(); ++i) {

        SceneLightData light = lights[i];

        glUniform3fv(glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].function").c_str()), 1, &light.function[0]);
        glUniform4fv(glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].color").c_str()), 1, &light.color[0]);

        GLuint type = glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].type").c_str());
        if (light.type != LightType::LIGHT_DIRECTIONAL)
        {
            glUniform4fv(glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].pos").c_str()), 1, &light.pos[0]);

            if (lights[i].type == LightType::LIGHT_SPOT)
            {
                glUniform1i(type, 3);
                glUniform1f(glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].angle").c_str()), light.angle);
                glUniform1f(glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].penumbra").c_str()), light.penumbra);
                glUniform4fv(glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].direction").c_str()), 1, &light.dir[0]);
            }
            else
            {
                glUniform1i(type, 2);
            }
        }
        else
        {
            glUniform1i(type, 1);
            glUniform4fv(glGetUniformLocation(m_phong_shader, ("lights[" + std::to_string(i) + "].direction").c_str()), 1, &light.dir[0]);
        }
    }
    // --------------- camera ------------------
    glm::vec4 pos = camera.pos;
    glUniform4f(glGetUniformLocation(m_phong_shader, "camera_pos"), pos[0], 10.0f - pos[1], pos[2], pos[3]);

    for (auto& p_s : phy_shapes)
    {
        if (p_s.shape.primitive.type == PrimitiveType::PRIMITIVE_CUBE || p_s.shape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
            continue;
        // Task 5: Generate a VBO here and store it in m_vbo
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        std::vector<float> vbo_data = p_s.vertexData;// generate_vbo(s);

        // Task 9: Pass the triangle vector into your VBO here
        glBufferData(GL_ARRAY_BUFFER, vbo_data.size() * sizeof(GLfloat), vbo_data.data(), GL_STATIC_DRAW);

        // ================== Vertex Array Objects

        // Task 11: Generate a VAO here and store it in m_vao
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // Task 13: Add position and color attributes to your VAO here
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat)));

        //glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "model_matrix"), 1, GL_FALSE, &s.ctm[0][0]);

        // -------------- material -----------------
        auto material = p_s.shape.primitive.material;
        glUniform4f(glGetUniformLocation(m_phong_shader, "material_ambient"), material.cAmbient[0], material.cAmbient[1], material.cAmbient[2], material.cAmbient[3]);
        glUniform4f(glGetUniformLocation(m_phong_shader, "material_diffuse"), material.cDiffuse[0], material.cDiffuse[1], material.cDiffuse[2], material.cDiffuse[3]);
        glUniform4f(glGetUniformLocation(m_phong_shader, "material_specular"), material.cSpecular[0], material.cSpecular[1], material.cSpecular[2], material.cSpecular[3]);
        glUniform1i(glGetUniformLocation(m_phong_shader, "shininess"), material.shininess);

        // reflection
        if (p_s.shape.primitive.type == PrimitiveType::PRIMITIVE_CUBE || p_s.shape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
        {
            glUniform1i(glGetUniformLocation(m_phong_shader, "is_water"), true);
        }
        else
        {
            glUniform1i(glGetUniformLocation(m_phong_shader, "is_water"), false);
        }

        // Draw Command
        glDrawArrays(GL_TRIANGLES, 0, vbo_data.size() / 8);

        // glDeleteBuffers(1, &m_vbo);
        // glDeleteVertexArrays(1, &m_vao);

    }

    //glBindVertexArray(0);
    // glBindTexture(GL_TEXTURE_2D, 0);
    // glBindVertexArray(0);
    glUseProgram(0);
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
    glViewport(0, 0, 2048, 2048);
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
    // // Task 24: Bind our FBO
    // glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // // Task 28: Call glViewport
    // glViewport(0, 0, m_fbo_width, m_fbo_height);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(0.5f, 0.1f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // starting to paint the texture
    // CUBEMAP ---------------------------------------------------------
    glDepthFunc(GL_LEQUAL);
    glUseProgram(m_skybox_shader);
    glm::mat4 view = glm::mat4(glm::mat3(camera.view_mat));

    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "projection"), 1, GL_FALSE, &camera.proj_mat[0][0]);
    glUniform1i(glGetUniformLocation(m_skybox_shader, "skybox"), 1);
    // skybox cube
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, fbo_tex_cube);
    glBindVertexArray(skyboxVAO);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDepthFunc(GL_LESS); // set depth function back to default
    glUseProgram(0);
    // CUBEMAP ---------------------------------------------------------

    glClearColor(0.1f, 1.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, fbo_tex_cube);

    paint_shapes(true);

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

    update(); // asks for a PaintGL() call to occur
}

std::vector<float> Realtime::generate_vertex_data(RenderShapeData s)
{
    std::vector<float> temp;
    if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE)
    {
        cube.updateParams(settings.shapeParameter1, 1.0f, 1.0f);
        temp = cube.generateShape();
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_CONE)
    {
        cone.updateParams(settings.shapeParameter1, settings.shapeParameter2, 1.0f, 1.0f);
        temp = cone.generateShape();
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
    {
        sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2, 1.0f, 1.0f);
        temp = sphere.generateShape();
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
    {
        cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2, 1.0f, 1.0f);
        temp = cylinder.generateShape();
    }

    for (int i = 0; i < temp.size() / 8; i++)
    {
        glm::vec3 world_pos = glm::vec3(s.ctm * glm::vec4(temp[i * 8], temp[i * 8 + 1], temp[i * 8 + 2], 1.0f));
        temp[i * 8] = world_pos[0];
        temp[i * 8 + 1] = world_pos[1];
        temp[i * 8 + 2] = world_pos[2];
        glm::vec3 world_normal = glm::normalize(glm::inverse(glm::transpose(glm::mat3(s.ctm))) * normalize(glm::vec3(temp[i * 8 + 3], temp[i * 8 + 4], temp[i * 8 + 5])));
        temp[i * 8 + 3] = world_normal[0];
        temp[i * 8 + 4] = world_normal[1];
        temp[i * 8 + 5] = world_normal[2];
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

void Realtime::ini_phy_shapes()
{
    for (auto &s : shapes)
    {
        if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
        {
            phy_shapes.push_back(physics_shape{true, glm::vec3(0.0f), glm::vec3(0.0f), rand_float(0.4f, 0.5f), generate_vertex_data(s), s});
        }
        else
        {
            phy_shapes.push_back(physics_shape{false, glm::vec3(0.0f), glm::vec3(0.0f), rand_float(0.4f, 0.5f), generate_vertex_data(s), s});
        }
    }
}

void Realtime::settingsChanged()
{
    if (settings.nearPlane != near_plane || settings.farPlane != far_plane)
    {
        near_plane = settings.nearPlane;
        far_plane = 1000.0f;
        camera.update(near_plane, far_plane);
    }

    if (settings.shapeParameter1 != parameter1 || settings.shapeParameter2 != parameter2 || (settings.adaptive_detail != adaptive_detail && !settings.adaptive_detail))
    {
        parameter1 = settings.shapeParameter1;
        parameter2 = settings.shapeParameter2;
        cone.updateParams(parameter1, parameter2, 1.0f, 1.0f);
        cube.updateParams(parameter1, 1.0f, 1.0f);
        cylinder.updateParams(parameter1, parameter2, 1.0f, 1.0f);
        sphere.updateParams(parameter1, parameter2, 1.0f, 1.0f);

        adaptive_detail = settings.adaptive_detail;
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

