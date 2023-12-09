#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include <ctime>
#include "settings.h"
#include "utils/shaderloader.h"

#include "glm/gtx/transform.hpp"

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat lastX = 400,lastY = 300;

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

//    Floor m_floor;



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

    m_floor.initialize();
    // fountain.initialize();

}

void Realtime::paintGL() {
//    printf("es");
    // Task 24: Bind our FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // Task 28: Call glViewport
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // paint_shapes();

    // Floor m_floor;
    m_floor.render(camera);

    glm::mat4 model = glm::mat4(1.0f);
    flame.initialize();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_screen_width/(float)m_screen_height,0.1f,2000.f);
    flame.Render(deltaTime,model,camera.view_mat,projection);

    //    GLfloat currentFrame = glfwGetTime();
    GLfloat currentFrame = (GLfloat)clock() / CLOCKS_PER_SEC;
//    std::cout << currentFrame << endl;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

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

    if (settings.adaptive_detail)
        calculate_adaptive_param();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged()
{
    if (settings.nearPlane != near_plane || settings.farPlane != far_plane)
    {
        near_plane = settings.nearPlane;
        far_plane = settings.farPlane;
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

    if (settings.adaptive_detail != adaptive_detail && settings.adaptive_detail)
    {
        adaptive_detail = settings.adaptive_detail;
        calculate_adaptive_param();
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
        blur_filter = settings.blur_filter;
    }

    if (settings.use_texture != use_texture)
    {
        use_texture = settings.use_texture;
    }

    update(); // asks for a PaintGL() call to occur
}

