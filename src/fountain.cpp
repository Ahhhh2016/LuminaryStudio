#include "fountain.h"


namespace Fountain{

Fountain::Fountain()
    {
    }

    Fountain::~Fountain()
    {
    }

    void Fountain::initialize(glm::vec3 pos) {
        changeCenter(pos);
        mCurVBOIndex = 0;
        mCurTransformFeedbackIndex = 1;
        mFirst = true;
        mTimer = 0;
        const GLchar* varyings[7] = { "Type1","Position1",
            "Velocity1","Age1","Size1", "Alpha1", "Life1"
        };//设置TransformFeedback要捕获的输出变量
        mUpdateShader = ShaderLoader::createShaderProgram(":/resources/shaders/Update.vs", ":/resources/shaders/Update.fs",
                                                          ":/resources/shaders/Update.gs", varyings, 7);
        //设置TransformFeedback缓存能够记录的顶点的数据类型

        mRenderShader = ShaderLoader::createShaderProgram(":/resources/shaders/Render.vs", ":/resources/shaders/Render.fs");
        //设置随机纹理
        InitRandomTexture(512);
        mSparkTexture.loadTexture("./resources/particle.bmp");

        glUseProgram(mRenderShader);
        // mRenderShader->use();
        // mRenderShader->sextInt("water", 0);
        glUniform1i(glGetUniformLocation(mRenderShader, "water"), 8);

        InitFountain();

        glUseProgram(0);
    }

    bool Fountain::InitFountain()
    {
        WaterParticle particles[MAX_PARTICLES];
        //WaterParticle particles = new WaterParticle[MAX_PARTICLES];
        memset(particles, 0, sizeof(particles));
        GenInitLocation(particles, INIT_PARTICLES);

        glGenTransformFeedbacks(2, mTransformFeedbacks);
        glGenBuffers(2, mParticleBuffers);
        glGenVertexArrays(2, mParticleArrays);
        for (int i = 0; i < 2; i++)
        {
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[i]);
            glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[i]);
            glBindVertexArray(mParticleArrays[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffers[i]);
        }
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        glBindVertexArray(0);
        //绑定纹理
//        mUpdateShader->use();
        glUseProgram(mUpdateShader);
        glBindTexture(GL_TEXTURE_1D, mRandomTexture);
//        mUpdateShader->setInt("gRandomTexture", 0);
        glUniform1i(glGetUniformLocation(mUpdateShader, "gRandomTexture"), 8);
        //delete[] particles;
        glUseProgram(0);
        return true;
    }

    void Fountain::Render(float frametimeMills, glm::mat4& worldMatrix,
                          glm::mat4& viewMatrix, glm::mat4& projectMatrix, glm::vec3& ds, float dis_factor)
    {
        glUseProgram(mUpdateShader);
        glUniform1f(glGetUniformLocation(mUpdateShader, "dis_factor"), dis_factor);
        glUseProgram(0);

        mTimer += frametimeMills*1000.0f;
        UpdateParticles(frametimeMills*1000.0f, ds);
        RenderParticles(worldMatrix, viewMatrix, projectMatrix, ds);
        mCurVBOIndex = mCurTransformFeedbackIndex;
        mCurTransformFeedbackIndex = (mCurTransformFeedbackIndex + 1) & 0x1;
    }

    void Fountain::UpdateParticles(float frametimeMills, glm::vec3& ds)
    {
        glUseProgram(mUpdateShader);
//        mUpdateShader->use();
//        mUpdateShader->setFloat("gDeltaTimeMillis", frametimeMills);
//        mUpdateShader->setFloat("gTime", mTimer);
//        mUpdateShader->setFloat("MAX_SIZE", MAX_SIZE);
//        mUpdateShader->setFloat("MIN_SIZE", MIN_SIZE);
//        mUpdateShader->setFloat("MAX_LAUNCH", MAX_LAUNCH);
//        mUpdateShader->setFloat("MIN_LAUNCH", MIN_LAUNCH);
//        mUpdateShader->setFloat("angle", ANGLE);
//        mUpdateShader->setFloat("R", radius);
//        mUpdateShader->setVec3("NORMAL", glm::vec3(0, 1, 0));
        glUniform1f(glGetUniformLocation(mUpdateShader, "gDeltaTimeMillis"), frametimeMills);
        glUniform1f(glGetUniformLocation(mUpdateShader, "gTime"), mTimer);
        glUniform1f(glGetUniformLocation(mUpdateShader, "MAX_SIZE"), MAX_SIZE);
        glUniform1f(glGetUniformLocation(mUpdateShader, "MIN_SIZE"), MIN_SIZE);
        glUniform3fv(glGetUniformLocation(mUpdateShader, "MAX_VELOC"), 1, &MAX_VELOC[0]);
        glUniform3fv(glGetUniformLocation(mUpdateShader, "MIN_VELOC"), 1, &MIN_VELOC[0]);
        glUniform1f(glGetUniformLocation(mUpdateShader, "MAX_LAUNCH"), MAX_LAUNCH);
        glUniform1f(glGetUniformLocation(mUpdateShader, "MIN_LAUNCH"), MIN_LAUNCH);
        glUniform1f(glGetUniformLocation(mUpdateShader, "angle"), ANGLE);
        glUniform1f(glGetUniformLocation(mUpdateShader, "R"), radius);

        glUniform3fv(glGetUniformLocation(mUpdateShader, "dsUniform"), 1, &ds[0]);


        glm::vec3 normal = glm::vec3(0.f, 1.f, 0.f);
        glUniform3fv(glGetUniformLocation(mUpdateShader, "NORMAL"), 1, &normal[0]);

        //绑定纹理
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_1D, mRandomTexture);

        glEnable(GL_RASTERIZER_DISCARD);//我们渲染到TransformFeedback缓存中去，并不需要光栅化
        glBindVertexArray(mParticleArrays[mCurVBOIndex]);
        glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurVBOIndex]);
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[mCurTransformFeedbackIndex]);

        glEnableVertexAttribArray(0);//type
        glEnableVertexAttribArray(1);//position
        glEnableVertexAttribArray(2);//velocity
        glEnableVertexAttribArray(3);//lifetime
        glEnableVertexAttribArray(4);//size
        glEnableVertexAttribArray(5);//alpha
        glEnableVertexAttribArray(6); //life
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, type));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, position));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, velocity));
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, lifetimeMills));
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, size));
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, alpha));
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, life));
        glBeginTransformFeedback(GL_POINTS);
        if (mFirst)
        {
            glDrawArrays(GL_POINTS, 0, INIT_PARTICLES);
            mFirst = false;
        }
        else {
            glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurVBOIndex]);
        }
        glEndTransformFeedback();
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(4);
        glDisableVertexAttribArray(5);
        glDisableVertexAttribArray(6);
        glDisable(GL_RASTERIZER_DISCARD);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        glUseProgram(0);
    }

    void Fountain::RenderParticles(glm::mat4& worldMatrix,
        glm::mat4& viewMatrix, glm::mat4& projectMatrix, glm::vec3& ds)
    {
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glUseProgram(mRenderShader);
//        mRenderShader->use();
//        mRenderShader->setMat4("model", worldMatrix);
//        mRenderShader->setMat4("view", viewMatrix);
//        mRenderShader->setMat4("projection", projectMatrix);
        glUniformMatrix4fv(glGetUniformLocation(mRenderShader, "model"), 1, GL_FALSE, &worldMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mRenderShader, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(mRenderShader, "projection"), 1, GL_FALSE, &projectMatrix[0][0]);

        glUniform3fv(glGetUniformLocation(mUpdateShader, "dsUniform"), 1, &ds[0]);

        //glBindVertexArray(mParticleArrays[mCurTransformFeedbackIndex]);
        //glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,mParticleBuffers[mCurTransformFeedbackIndex]);
        glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurTransformFeedbackIndex]);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, position));
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, size));
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(WaterParticle), (void*)offsetof(WaterParticle, alpha));

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, mSparkTexture.textureID);
        glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurTransformFeedbackIndex]);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }

    void Fountain::InitRandomTexture(unsigned int size)
    {
        srand(time(NULL));
        glm::vec3* pRandomData = new glm::vec3[size];
        for (int i = 0; i < size; i++)
        {
            pRandomData[i].x = float(rand()) / float(RAND_MAX);
            pRandomData[i].y = float(rand()) / float(RAND_MAX);
            pRandomData[i].z = float(rand()) / float(RAND_MAX);
        }
        glGenTextures(1, &mRandomTexture);
        glBindTexture(GL_TEXTURE_1D, mRandomTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0, GL_RGB, GL_FLOAT, pRandomData);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        delete[] pRandomData;
        pRandomData = nullptr;
    }

    void Fountain::GenInitLocation(WaterParticle particles[], int nums) {
//        srand(time(NULL));
//        for (int x = 0; x < nums; x++) {
//            glm::vec3 record(0.0f);
//            record.x = (2.0f*float(rand()) / float(RAND_MAX) - 1.0f)*radius;
//            record.z = (2.0f*float(rand()) / float(RAND_MAX) - 1.0f)*radius;
//            while (sqrt(record.x*record.x + record.z*record.z)>radius) {
//                record.x = (2.0f*float(rand()) / float(RAND_MAX) - 1.0f)*radius;
//                record.z = (2.0f*float(rand()) / float(RAND_MAX) - 1.0f)*radius;
//            }
//            record.y = 0.0f;
//            particles[x].type = PARTICLE_TYPE_LAUNCHER;
//            particles[x].position = record;
//            particles[x].velocity = glm::vec3(0.0f);
//            particles[x].size = INIT_SIZE;//发射器粒子大小
//            particles[x].lifetimeMills = (MAX_LAUNCH-MIN_LAUNCH)*(float(rand()) / float(RAND_MAX)) + MIN_LAUNCH;
//        }

        srand(time(NULL));
        int n = 10;
        float Adj_value = 0.05f;
        // float radius = 0.7f;//火焰地区半径
        for (int x = 0; x < nums; x++) {
            glm::vec3 record(0.0f);
            record.x = center.x;
            record.y = center.y;
            record.z = center.z;
            for (int y = 0; y < n; y++) {//生成高斯分布的粒子，中心多，外边少
                record.x += (2.0f*float(rand()) / float(RAND_MAX) - 1.0f);
                record.z += (2.0f*float(rand()) / float(RAND_MAX) - 1.0f);
                record.y += (2.0f*float(rand()) / float(RAND_MAX) - 1.0f);
            }
            record.x *= radius;
            record.z *= radius;
            record.y *= radius;
            particles[x].type = PARTICLE_TYPE_LAUNCHER;
            particles[x].position = record;
            particles[x].velocity = DEL_VELOC*(float(rand()) / float(RAND_MAX)) + MIN_VELOC;//在最大最小速度之间随机选择
            particles[x].alpha = 0.6f;
            particles[x].size = INIT_SIZE;//发射器粒子大小
                //在最短最长寿命之间随机选择
            particles[x].lifetimeMills = (MAX_LAUNCH - MIN_LAUNCH)*(float(rand()) / float(RAND_MAX)) + MIN_LAUNCH;
            float dist = sqrt(record.x*record.x + record.z*record.z);
            particles[x].life = particles[x].lifetimeMills;
        }
    }

    void Fountain::changeCenter(glm::vec3 pos) {
        center.x = pos.x;
        center.y = pos.y;
        center.z = pos.z;

//        InitFountain();
//        WaterParticle particles[MAX_PARTICLES];
//        memset(particles, 0, sizeof(particles));
//        GenInitLocation(particles, INIT_PARTICLES);
    }


}
