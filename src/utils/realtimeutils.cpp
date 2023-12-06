#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"

void Realtime::paint_skybox()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw skybox as last
    glDepthFunc(GL_LEQUAL);
    glUseProgram(m_skybox_shader);
    glm::mat4 view = glm::mat4(glm::mat3(camera.view_mat));

    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "projection"), 1, GL_FALSE, &camera.proj_mat[0][0]);
    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
    glUseProgram(0);
}


void Realtime::paint_shapes() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_phong_shader);

    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "view_matrix"), 1, GL_FALSE, &camera.view_mat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "projection_matrix"), 1, GL_FALSE, &camera.proj_mat[0][0]);
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
    glUniform4f(glGetUniformLocation(m_phong_shader, "camera_pos"), pos[0], pos[1], pos[2], pos[3]);

    for (auto& p_s : phy_shapes)
    {
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

        // -------------- texture -----------------
        auto texture_map = material.textureMap;
        if (settings.use_texture && texture_map.isUsed)
        {
            QString texture_filepath = QString::fromStdString(texture_map.filename);
            m_image = QImage(texture_filepath);
            m_image = m_image.convertToFormat(QImage::Format_RGBA8888).mirrored();

            glBindVertexArray(m_vao);
            glGenTextures(1, &m_shape_texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_shape_texture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glUniform1i(glGetUniformLocation(m_phong_shader, "object_texture"), 1);
            glUniform1f(glGetUniformLocation(m_phong_shader, "blend"), float(material.blend));
            glUniform1i(glGetUniformLocation(m_phong_shader, "use_texture"), true);
        }
        else
        {
            glUniform1i(glGetUniformLocation(m_phong_shader, "use_texture"), false);
        }


        // Draw Command
        glDrawArrays(GL_TRIANGLES, 0, vbo_data.size() / 8);

        glDeleteBuffers(1, &m_vbo);
        glDeleteVertexArrays(1, &m_vao);

    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

std::vector<float> Realtime::generate_vbo(RenderShapeData s)
{
    auto texture = s.primitive.material.textureMap;
    if (adaptive_detail)
    {
        glm::vec4 sample_point(0.0f, 0.0f, 0.0f, camera.pos[3]);
        float dis = glm::distance(camera.pos, s.ctm * sample_point);

        calculate_adaptive_param();
        float level = distanceK * dis + distanceB;

        if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE)
            cube.updateParams(ceil(settings.shapeParameter1 * level), texture.repeatU, texture.repeatV);
        else if (s.primitive.type == PrimitiveType::PRIMITIVE_CONE)
            cone.updateParams(ceil(settings.shapeParameter1 * level), ceil(settings.shapeParameter2 * level), texture.repeatU, texture.repeatV);
        else if (s.primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
            sphere.updateParams(ceil(settings.shapeParameter1 * level), ceil(settings.shapeParameter2 * level), texture.repeatU, texture.repeatV);
        else if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
            cylinder.updateParams(ceil(settings.shapeParameter1 * level), ceil(settings.shapeParameter2 * level), texture.repeatU, texture.repeatV);
    }

    if (s.primitive.type == PrimitiveType::PRIMITIVE_CUBE)
    {
        cube.updateParams(settings.shapeParameter1, texture.repeatU, texture.repeatV);
        return cube.generateShape();
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_CONE)
    {
        cone.updateParams(settings.shapeParameter1, settings.shapeParameter2, texture.repeatU, texture.repeatV);
        return cone.generateShape();
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_SPHERE)
    {
        sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2, texture.repeatU, texture.repeatV);
        return sphere.generateShape();
    }
    else if (s.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER)
    {
        cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2, texture.repeatU, texture.repeatV);
        return cylinder.generateShape();
    }

    return std::vector<float>{0.0f};
}

void Realtime::calculate_adaptive_param()
{
    float max_dis = -1.0f, min_dis = std::numeric_limits<float>::max();
    for (auto &s : shapes)
    {
        glm::vec4 sample_point(0.0f, 0.0f, 0.0f, camera.pos[3]);
        float dis = glm::distance(camera.pos, s.ctm * sample_point);

        if (dis > max_dis)
            max_dis = dis;
        if (dis < min_dis)
            min_dis = dis;
    }
    distanceK = 3.0f / (4.0f * (min_dis - max_dis));
    distanceB = 1 - distanceK * min_dis;
}

void Realtime::makeFBO()
{
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8 , m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
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
