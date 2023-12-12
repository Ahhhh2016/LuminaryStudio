#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/gtx/transform.hpp"

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat dTime = 0.0f;
GLfloat lFrame = 0.0f;

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
//    unsigned char *data2 = stbi_load(("./resources/SelectedPaper/paper1.jpg"), &wid, &hei, &nrComponents, 0);
    QString paper_filepath = QString(":/resources/SelectedPaper/paper3.jpg");
    m_image = QImage(paper_filepath);
    m_image = m_image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    //glBindVertexArray(m_vao);
    glGenTextures(1, &m_shape_texture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_shape_texture);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wid, hei, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());

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
                glUniform1i(glGetUniformLocation(m_phong_shader, "use_texture"), true);;
                glUniform1f(glGetUniformLocation(m_phong_shader, "radius_light"), radius_light);
                glUniform2fv(glGetUniformLocation(m_phong_shader, "light_offset"), 1, &offset[0]);

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
//        printf("hhh\n");

    }

    for (auto& p_s : phy_shapes)
    {
        if (!(p_s.shape[0].primitive.type == PrimitiveType::PRIMITIVE_MESH))
            continue;

        for (int i=0; i < p_s.shape.size(); i++)
        {
            glm::vec3 delta(p_s.bottom_center / 1.72f);

            glm::mat4 ctm  = glm::translate(glm::vec3(0.0f, 2.0f, 0.0f) + p_s.shape_rand_vec) * glm::scale(glm::vec3(0.5f, 0.5f, 0.5f)) * p_s.shape[0].ctm;

            m_fountain[p_s.flame_index].Render(dTime, ctm, camera.view_mat, camera.proj_mat, delta);
        }
    }

    //    GLfloat currentFrame = glfwGetTime();
    GLfloat currentFrame = (GLfloat)clock() / CLOCKS_PER_SEC;
    //    std::cout << currentFrame << endl;
    dTime = currentFrame - lFrame;
    lFrame = currentFrame;

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

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
