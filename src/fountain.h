#ifndef FOUNTAIN_H
#define FOUNTAIN_H
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>

#include <iostream>
#include <ctime>
#include <math.h>
#include <glm/glm.hpp>
#include "camera/camera.h"
#include "utils/shaderloader.h"
#include "texture.h"
//#include "shader.h"



namespace Fountain {

    #define PARTICLE_TYPE_LAUNCHER 0.0f
    #define PARTICLE_TYPE_SHELL 1.0f
        //最大速度
    #define MAX_VELOC glm::vec3(0.0,10.0,0.0)
        //最小速度
    #define MIN_VELOC glm::vec3(0.0,7.0,0.0)
    //最大最小速度差距
    #define DEL_VELOC glm::vec3(0.0,3.0,0.0)
        //发射粒子最大周期
    #define MAX_LAUNCH 0.3f*1000.0f
        //发射粒子最小周期
    #define MIN_LAUNCH 0.05f*1000.0f
        //初始点精灵大小
    #define INIT_SIZE 1.0f
    #define MAX_SIZE 1.0f
    #define MIN_SIZE 1.0f

    const float ANGLE = 120.0f;
    const int MAX_PARTICLES = 2000;//定义粒子发射系统最大的粒子数

    //初始发射器例子数量
    const int INIT_PARTICLES = 1000;

    //落雪中心
//    const glm::vec3 center(0.0f);
    const float radius = 0.3f;

    struct WaterParticle
    {
        float type;
        glm::vec3 position;
        glm::vec3 velocity;
        float lifetimeMills;//年龄
        float size;//粒子点精灵大小
        float alpha;
        float life; //寿命
    };

    class Fountain
    {
    public:
        Fountain();
        ~Fountain();
        void initialize(glm::vec3 pos);
        void Render(float frametimeMills, glm::mat4& worldMatrix, glm::mat4& viewMatrix, glm::mat4& projectMatrix, glm::vec3& ds);
        void changeCenter(glm::vec3 pos);
    private:
        // flame center
        glm::vec3 center;

        bool InitFountain();
        void UpdateParticles(float frametimeMills, glm::vec3& ds);//更新粒子的位置等
        void InitRandomTexture(unsigned int size);//生成1维随机纹理
        void RenderParticles(glm::mat4& worldMatrix, glm::mat4& viewMatrix, glm::mat4& projectMatrix, glm::vec3& ds);
        void GenInitLocation(WaterParticle partciles[], int nums);//生成初始粒子

        unsigned int mCurVBOIndex, mCurTransformFeedbackIndex;
        GLuint mParticleBuffers[2]; //粒子发射系统的两个顶点缓存区
        GLuint mParticleArrays[2];
        GLuint mTransformFeedbacks[2];//粒子发射系统对应的TransformFeedback
        GLuint mRandomTexture;//随机一维纹理
        CTexture mSparkTexture;//Alpha纹理
        CTexture mStartTexture;
        float mTimer;//粒子发射器已经发射的时间
        bool mFirst;
        GLuint mUpdateShader;//更新粒子的GPUProgram
        GLuint mRenderShader;//渲染粒子的GPUProgram
    };

}


#endif // FOUNTAIN_H
