#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool Realtime::loadCubeMapSide(GLuint texture, GLenum side_target, std::string file_name) {
    int x, y, n;
    int force_channels = 4;
    unsigned char*  image_data = stbi_load(
        file_name.c_str(), &x, &y, &n, force_channels);
    if (!image_data) {
        std::cerr << "ERROR: could not load " << file_name << std::endl;
        return false;
    }
    // non-power-of-2 dimensions check
    if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
        std::cerr << "WARNING: image " << file_name << " is not power-of-2 dimensions " << std::endl;
    }

    skybox_width = x;
    skybox_height = y;

    // copy image data into 'target' side of cube map
    glTexImage2D(
        side_target,
        0,
        GL_RGBA,
        x,
        y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        image_data);
    free(image_data);
    return true;
}

// void Realtime::ini_skybox()
// {
//     for (float& i : skyboxVertices) {
//         i *= 100;
//     }

//     // skybox VAO
//     // skybox
//     glEnable(GL_DEPTH_TEST);
//     m_skybox_shader = ShaderLoader::createShaderProgram(":/resources/shaders/skybox.vert", ":/resources/shaders/skybox.frag");

//     glGenVertexArrays(1, &skyboxVAO);
//     glGenBuffers(1, &skyboxVBO);
//     glBindVertexArray(skyboxVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
//     glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size()*sizeof(GLfloat), skyboxVertices.data(), GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);

//     // load textures
//     // -------------
//     std::vector<std::string> faces
//         {
//             "./resources/skybox/right.jpg",
//             "./resources/skybox/left.jpg",
//             "./resources/skybox/top.jpg",
//             "./resources/skybox/bottom.jpg",
//             "./resources/skybox/front.jpg",
//             "./resources/skybox/back.jpg",
//         };

//     //cubemapTexture = loadCubemap(faces);
//     glGenTextures(1, &cubemapTexture);
//     // binding
//     glActiveTexture(GL_TEXTURE0);
//     glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

//     // load each image and copy into a side of the cube-map texture
//     loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, faces[4]);
//     loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, faces[5]);
//     loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, faces[2]);
//     loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, faces[3]);
//     loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, faces[1]);
//     loadCubeMapSide(cubemapTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_X, faces[0]);
//     // format cube map texture
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//     // unbinding
//     glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

//     // init dynamic cubemap
//     // depth buffer
//     glGenRenderbuffers(1, &fbo_rb_cube);
//     glBindRenderbuffer(GL_RENDERBUFFER, fbo_rb_cube);
//     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, skybox_width, skybox_height);
//     glBindRenderbuffer(GL_RENDERBUFFER, 0);

//     // texture
//     // create the cubemap
//     glGenTextures(1, &fbo_tex_cube);
//     glActiveTexture(GL_TEXTURE1); // texture slot 1
//     glBindTexture(GL_TEXTURE_CUBE_MAP, fbo_tex_cube);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     for (int i = 0; i < 6; ++i) {
//         glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, skybox_width, skybox_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
//     }
//     glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

//     glGenFramebuffers(1, &fbo_cube);
//     glBindFramebuffer(GL_FRAMEBUFFER, fbo_cube);
//     // attach buffer & tex to fbo
//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_rb_cube);
//     glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_tex_cube, 0);
//     glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

// }

// GLuint Realtime::loadCubemap(std::vector<std::string> faces)
// {
//     GLuint textureID;
//     glGenTextures(1, &textureID);
//     glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

//     int width, height, nrComponents;
//     for (unsigned int i = 0; i < faces.size(); i++)
//     {
//         unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
//         if (data)
//         {
//             glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//             stbi_image_free(data);
//         }
//         else
//         {
//             std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
//             stbi_image_free(data);
//         }
//     }

//     cubemap_size = width;

//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

//     return textureID;
// }

void Realtime::init_shapes()
{
    glUseProgram(m_phong_shader);

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

    glUseProgram(0);

    makeCurrent();

    int wid, hei, nrComponents;
    unsigned char *data = stbi_load("./resources/waterDUDV.png", &wid, &hei, &nrComponents, 0);
    //glBindVertexArray(m_vao);
    glGenTextures(1, &m_shape_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_shape_texture);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wid, hei, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // nonrmalMap
    unsigned char *data1 = stbi_load("./resources/normalMap.png", &wid, &hei, &nrComponents, 0);
    //glGenVertexArrays(1, &m_vao_normal);
    //glBindVertexArray(m_vao);
    glGenTextures(1, &m_normal_texture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_normal_texture);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wid, hei, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // fire texture
    int wid2, hei2, nrComponents2;

    //unsigned char *data = stbi_load(("./resources/lanternTextures/Flame_BottomPNG/Flame_Bottom." + ss.str() + ".png").c_str(), &wid, &hei, &nrComponents, 0);
    QString paper_filepath = QString(":/resources/paper3.jpg");
    m_image = QImage(paper_filepath);
    m_image = m_image.convertToFormat(QImage::Format_RGBA8888).mirrored();

    unsigned char *data2 = stbi_load(("./resources/paper.png"), &wid, &hei, &nrComponents, 0);
    //glBindVertexArray(m_vao);
    glGenTextures(1, &m_shape_texture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_shape_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wid, hei, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    doneCurrent();
}



void Realtime::paint_shapes(bool paint_all, Camera c) {

    glUseProgram(m_phong_shader);

    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "view_matrix"), 1, GL_FALSE, &c.view_mat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "projection_matrix"), 1, GL_FALSE, &c.proj_mat[0][0]);

    // --------------- camera ------------------
    glm::vec4 pos = c.pos;
    glUniform4f(glGetUniformLocation(m_phong_shader, "camera_pos"), pos[0], pos[1], pos[2], pos[3]);

    for (auto& p_s : phy_shapes)
    {
        for (int i=0; i < p_s.shape.size(); i++)
        {
            if (!paint_all && p_s.apply_reflection)
                continue;

            if (p_s.apply_reflection)
            {
                glUniform1i(glGetUniformLocation(m_phong_shader, "apply_reflection"), true);
            }
            else
            {
                glUniform1i(glGetUniformLocation(m_phong_shader, "apply_reflection"), false);
            }
            // Task 5: Generate a VBO here and store it in m_vbo
            glGenBuffers(1, &m_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

            std::vector<float> vbo_data = p_s.vertexData[i];// generate_vbo(s);

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
            auto material = p_s.shape[i].primitive.material;
            glUniform4f(glGetUniformLocation(m_phong_shader, "material_ambient"), material.cAmbient[0], material.cAmbient[1], material.cAmbient[2], material.cAmbient[3]);
            glUniform4f(glGetUniformLocation(m_phong_shader, "material_diffuse"), material.cDiffuse[0], material.cDiffuse[1], material.cDiffuse[2], material.cDiffuse[3]);
            glUniform4f(glGetUniformLocation(m_phong_shader, "material_specular"), material.cSpecular[0], material.cSpecular[1], material.cSpecular[2], material.cSpecular[3]);
            glUniform1i(glGetUniformLocation(m_phong_shader, "shininess"), material.shininess);

            // -------------- texture -----------------
            if (p_s.shape[0].primitive.type == PrimitiveType::PRIMITIVE_MESH && i == 0)
            {
                //std::stringstream ss;
                //ss << std::setw(5) << std::setfill('0') << fire_image_num;

                glUniform1i(glGetUniformLocation(m_phong_shader, "object_texture"), 4);
                glUniform1f(glGetUniformLocation(m_phong_shader, "blend"), 1.0);
                glUniform1i(glGetUniformLocation(m_phong_shader, "use_texture"), true);
            }
            else
            {
                glUniform1i(glGetUniformLocation(m_phong_shader, "use_texture"), false);
            }

            // reflection
            if (p_s.is_water)
            {
                glUniform1i(glGetUniformLocation(m_phong_shader, "is_water"), true);

                if (paint_all)
                {
                    glUniform1f(glGetUniformLocation(m_phong_shader, "moveFactor"), d_time);
                    // dudvMap

                }
            }
            else
            {
                glUniform1i(glGetUniformLocation(m_phong_shader, "is_water"), false);
            }

            // Draw Command
            glDrawArrays(GL_TRIANGLES, 0, vbo_data.size() / 8);

            glDeleteBuffers(1, &m_vbo);
            glDeleteVertexArrays(1, &m_vao);

        }
    }


    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

// std::vector<float> Realtime::generate_vbo(RenderShapeData s)
// {
//     auto texture = s.primitive.material.textureMap;

//     if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE)
//     {
//         cube.updateParams(settings.shapeParameter1, texture.repeatU, texture.repeatV);
//         return cube.generateShape();
//     }
//     else if (s.primitive.type == PrimitiveType::PRIMITIVE_CONE)
//     {
//         cone.updateParams(settings.shapeParameter1, settings.shapeParameter2, texture.repeatU, texture.repeatV);
//         return cone.generateShape();
//     }
//     else if (s.primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
//     {
//         sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2, texture.repeatU, texture.repeatV);
//         return sphere.generateShape();
//     }
//     else if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
//     {
//         cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2, texture.repeatU, texture.repeatV);
//         return cylinder.generateShape();
//     }

//     return std::vector<float>{0.0f};
// }

void Realtime::paintTexture(GLuint texture)
{
    glUseProgram(m_postprocess_shader);
    // Task 32: Set your bool uniform on whether or not to filter the texture drawn
    glUniform1i(glGetUniformLocation(m_postprocess_shader, "invert_filter"), invert_filter);
    glUniform1i(glGetUniformLocation(m_postprocess_shader, "sharpen_filter"), sharpen_filter);
    glUniform1i(glGetUniformLocation(m_postprocess_shader, "grayscale_filter"), grayscale_filter);
    glUniform1i(glGetUniformLocation(m_postprocess_shader, "blur_filter"), blur_filter);

    glUniform1i(glGetUniformLocation(m_postprocess_shader, "width"), m_fbo_width);
    glUniform1i(glGetUniformLocation(m_postprocess_shader, "height"), m_fbo_height);

    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
void Realtime::makeFullscreenVAO() {
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    //
         -1.f,  1.f, 0.0f,
         0.0f, 1.f, 0.0f,
         -1.f, -1.f, 0.0f,
         0.0f, 0.0f, 0.0f,
         1.f, -1.f, 0.0f,
         1.f, 0.f, 0.f,
         1.f, 1.f, 0.0f,
         1.f, 1.f, 0.0f,
         -1.f, 1.f, 0.0f,
         0.f, 1.f, 0.0f,
         1.f, -1.f, 0.0f,
         1.f, 0.f, 0.f,
         };
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);
    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void Realtime::makePingpong() {

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongTexture);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screen_width, m_screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTexture[i], 0);

        GLuint fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Ping-Pong Framebuffer error: " << fboStatus << std::endl;
    }
}
// this is the fbo&texture&renderbuffer that used to keep the result of phong ,as the last step before transfer all these data to postprocess shader
void Realtime::makePhongFBO() {
    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_phong_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_phong_fbo);

    glGenTextures(1, &basicTexture);
    glBindTexture(GL_TEXTURE_2D, basicTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, basicTexture, 0);


    glGenTextures(1, &bloomTexture);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomTexture, 0);

    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_phong_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_phong_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_FRAMEBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_phong_renderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Realtime::paintBlur() {
    bool first_iteration = true;
    // Amount of time to bounce the blur
    int amount = 6;
    glUseProgram(m_blur_shader);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_screen_width , m_screen_height);
    //    glEnable(GL_DEPTH_TEST | GL_CULL_FACE);
    glClearColor(0.0, 1.0, 1.0, 1.0);
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glViewport(0, 0, m_screen_width , m_screen_height);
        glUniform1i(glGetUniformLocation(m_blur_shader, "horizontal"), horizontal);

        // In the first bounc we want to get the data from the bloomTexture
        if (first_iteration)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, bloomTexture);
            first_iteration = false;
        }
        // Move the data between the pingPong textures
        else
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pingpongTexture[!horizontal]);
        }

        // Render the image
        glBindVertexArray(m_fullscreen_vao);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Switch between vertical and horizontal blurring
        horizontal = !horizontal;
    }
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);//both 0 / m_defaultFBO work
}

void Realtime::paintPost() {
    //    makeCurrent();
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width , m_screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST | GL_CULL_FACE);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glUseProgram(m_postprocess_shader);
    glBindVertexArray(m_fullscreen_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, basicTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture/*pingpongTexture[!horizontal]*/);
    glDrawArrays(GL_TRIANGLES, 0, 6);// modify here
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //!!!!!!!!must be 0, if it's m_default_fbo, show nothing
    //    doneCurrent();
}
