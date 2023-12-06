#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "utils/sceneparser.h"
#include "utils/scenefilereader.h"
#include "utils/scenedata.h"
#include "camera/camera.h"

#include "shapes/Cone.h"
#include "shapes/Cube.h"
#include "shapes/Cylinder.h"
#include "shapes/Sphere.h"

struct physics_shape {
    bool apply_physics;
    glm::vec3 v;
    glm::vec3 f;
    float m;
    std::vector<float> vertexData;
    RenderShapeData shape;
};

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);
    void transfer_lights(std::vector<SceneLightData> lights);
    void transfer_data(RenderShapeData s);
    std::vector<float> generate_vbo(RenderShapeData s);
    void calculate_adaptive_param();

    void makeFBO();
    void paintTexture(GLuint texture);

    void paint_shapes(bool paint_all);
    void paint_skybox();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    // Project 5
    SceneGlobalData globalData;
    SceneCameraData cameraData;
    Camera camera;
    std::vector<SceneLightData> lights;
    std::vector<RenderShapeData> shapes;

    int parameter1;
    int parameter2;
    float near_plane;
    float far_plane;

    // adaptive detail (project 5 extra credit)
    bool adaptive_detail = false;
    float distanceK;
    float distanceB;

    // fbo filter (project 6)
    bool sharpen_filter = false;
    bool invert_filter = false;
    bool grayscale_filter = false;
    bool blur_filter = false;

    Cone cone;
    Cube cube;
    Cylinder cylinder;
    Sphere sphere;

    GLuint m_phong_shader; // Stores id of default shader program
    GLuint m_postprocess_shader; // Stores id of texture shader program
    GLuint m_vbo;    // Stores id of VBO
    GLuint m_vao;    // Stores id of VAO

    // Project 6
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_defaultFBO;
    GLuint m_fullscreen_vao;
    GLuint m_fullscreen_vbo;
    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;

    //texture
    QImage m_image;
    GLuint m_shape_texture;
    bool use_texture = false;

    bool stop = false;

    //lantern
    std::vector<physics_shape> phy_shapes;
    glm::vec3 wind = glm::vec3(-0.3f, 0.0f, -0.3f);
    float b = 0.35;
    float lift_force = 5.5f;
    void ini_phy_shapes();
    std::vector<float> generate_vertex_data(RenderShapeData s);
    void update_phy_shape(float dt);
    float rand_float(float min_float, float max_float);

    // dynamic environment mapping
    Camera topCamera;
    Camera bottomCamera;
    Camera leftCamera;
    Camera rightCamera;
    Camera frontCamera;
    Camera backCamera;
    void drawFboSide(Camera c);
    GLuint fbo_tex_cube, fbo_rb_cube, fbo_cube;

    // skybox
    GLuint m_skybox_shader;
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
    void ini_skybox();

    std::vector<GLfloat> skyboxVertices = {
        // positions
        -10.0f,  10.0f, -10.0f,
        -10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,

        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,

        -10.0f, -10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,

        -10.0f,  10.0f, -10.0f,
        10.0f,  10.0f, -10.0f,
        10.0f,  10.0f,  10.0f,
        10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
        10.0f, -10.0f, -10.0f,
        10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
        10.0f, -10.0f,  10.0f
    };
};
