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
        //����ٶ�
    #define MAX_VELOC glm::vec3(0.0,10.0,0.0)
        //��С�ٶ�
    #define MIN_VELOC glm::vec3(0.0,7.0,0.0)
    //�����С�ٶȲ��
    #define DEL_VELOC glm::vec3(0.0,3.0,0.0)
        //���������������
    #define MAX_LAUNCH 0.3f*1000.0f
        //����������С����
    #define MIN_LAUNCH 0.05f*1000.0f
        //��ʼ�㾫���С
    #define INIT_SIZE 1.0f
    #define MAX_SIZE 1.0f
    #define MIN_SIZE 1.0f

    const float ANGLE = 120.0f;
    const int MAX_PARTICLES = 2000;//�������ӷ���ϵͳ����������

    //��ʼ��������������
    const int INIT_PARTICLES = 1000;

    //��ѩ����
//    const glm::vec3 center(0.0f);
    const float radius = 0.3f;

    struct WaterParticle
    {
        float type;
        glm::vec3 position;
        glm::vec3 velocity;
        float lifetimeMills;//����
        float size;//���ӵ㾫���С
        float alpha;
        float life; //����
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
        void UpdateParticles(float frametimeMills, glm::vec3& ds);//�������ӵ�λ�õ�
        void InitRandomTexture(unsigned int size);//����1ά�������
        void RenderParticles(glm::mat4& worldMatrix, glm::mat4& viewMatrix, glm::mat4& projectMatrix, glm::vec3& ds);
        void GenInitLocation(WaterParticle partciles[], int nums);//���ɳ�ʼ����

        unsigned int mCurVBOIndex, mCurTransformFeedbackIndex;
        GLuint mParticleBuffers[2]; //���ӷ���ϵͳ���������㻺����
        GLuint mParticleArrays[2];
        GLuint mTransformFeedbacks[2];//���ӷ���ϵͳ��Ӧ��TransformFeedback
        GLuint mRandomTexture;//���һά����
        CTexture mSparkTexture;//Alpha����
        CTexture mStartTexture;
        float mTimer;//���ӷ������Ѿ������ʱ��
        bool mFirst;
        GLuint mUpdateShader;//�������ӵ�GPUProgram
        GLuint mRenderShader;//��Ⱦ���ӵ�GPUProgram
    };

}


#endif // FOUNTAIN_H
