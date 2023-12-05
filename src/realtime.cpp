#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"

#include "glm/gtx/transform.hpp"

// ================== Project 5: Lights, Camera

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent), camera()
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_phong_shader = ShaderLoader::createShaderProgram(":/resources/shaders/phong.vert", ":/resources/shaders/phong.frag");
    m_postprocess_shader = ShaderLoader::createShaderProgram(":/resources/shaders/postprocess.vert", ":/resources/shaders/postprocess.frag");

    //==============project 6================
    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Task 10: Set the texture.frag uniform for our texture
    glUseProgram(m_postprocess_shader);
    glUniform1i(glGetUniformLocation(m_postprocess_shader, "u_texture"), 0);
    glUseProgram(0);

    // Task 13: Add UV coordinates
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    //    UV   //
            -1.0f,  1.0f, 0.0f,  0.0f,1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f,0.0f,
            1.0f, -1.0f, 0.0f,  1.0f,0.0f,
            1.0f,  1.0f, 0.0f,  1.0f,1.0f,
            -1.0f,  1.0f, 0.0f,  0.0f,1.0f,
            1.0f, -1.0f, 0.0f,  1.0f,0.0f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeFBO();

    //==============project 6================
}

void Realtime::paintGL() {
    // Task 24: Bind our FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // Task 28: Call glViewport
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    paint_shapes();

    // Task 25: Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width * this->devicePixelRatio(), m_screen_height * this->devicePixelRatio());

    // Task 26: Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Task 27: Call paintTexture to draw our FBO color attachment texture | Task 31: Set bool parameter to true
    paintTexture(m_fbo_texture);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    camera.aspect_ratio = float(w) / float(h);
    camera.update(settings.nearPlane, settings.farPlane);

    //================project 6================
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Task 34: Regenerate your FBOs
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    makeFBO();
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

    //update shapes
    parameter1 = settings.shapeParameter1;
    parameter2 = settings.shapeParameter2;
    use_texture = settings.use_texture;
    adaptive_detail = settings.adaptive_detail;
    cone.updateParams(parameter1, parameter2, 1.0f, 1.0f);
    cube.updateParams(parameter1, 1.0f, 1.0f);
    cylinder.updateParams(parameter1, parameter2, 1.0f, 1.0f);
    sphere.updateParams(parameter1, parameter2, 1.0f, 1.0f);

    //generate phy shapes
    ini_phy_shapes();

    if (settings.adaptive_detail)
        calculate_adaptive_param();

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

void Realtime::ini_phy_shapes()
{
    for (auto &s : shapes)
    {
        if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
        {
            phy_shapes.push_back(physics_shape{true, glm::vec3(0.0f), glm::vec3(0.0f), 0.5f, generate_vertex_data(s), s});
        }
        else
        {
            phy_shapes.push_back(physics_shape{false, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, generate_vertex_data(s), s});
        }
    }
}

void Realtime::settingsChanged()
{
//    if (settings.nearPlane != near_plane || settings.farPlane != far_plane)
//    {
//        near_plane = settings.nearPlane;
//        far_plane = settings.farPlane;
//        camera.update(near_plane, far_plane);
//    }

//    if (settings.shapeParameter1 != parameter1 || settings.shapeParameter2 != parameter2 || (settings.adaptive_detail != adaptive_detail && !settings.adaptive_detail))
//    {
//        parameter1 = settings.shapeParameter1;
//        parameter2 = settings.shapeParameter2;
//        cone.updateParams(parameter1, parameter2, 1.0f, 1.0f);
//        cube.updateParams(parameter1, 1.0f, 1.0f);
//        cylinder.updateParams(parameter1, parameter2, 1.0f, 1.0f);
//        sphere.updateParams(parameter1, parameter2, 1.0f, 1.0f);

//        adaptive_detail = settings.adaptive_detail;
//    }

//    if (settings.adaptive_detail != adaptive_detail && settings.adaptive_detail)
//    {
//        adaptive_detail = settings.adaptive_detail;
//        calculate_adaptive_param();
//    }

//    if (settings.sharpen_filter != sharpen_filter)
//    {
//        sharpen_filter = settings.sharpen_filter;
//    }

//    if (settings.invert_filter != invert_filter)
//    {
//        invert_filter = settings.invert_filter;
//    }

//    if (settings.grayscale_filter != grayscale_filter)
//    {
//        grayscale_filter = settings.grayscale_filter;
//    }

//    if (settings.blur_filter != blur_filter)
//    {
//        blur_filter = settings.blur_filter;
//    }

//    if (settings.use_texture != use_texture)
//    {
//        use_texture = settings.use_texture;
//    }

    update(); // asks for a PaintGL() call to occur
}

