#pragma once
#include <GL/glew.h>
#include <iostream>
#include <ctime>
#include <math.h>
#include <glm/glm.hpp>
#include "camera/camera.h"
#include "utils/shaderloader.h"
#include "texture.h"
#include "shader.h"


namespace Fountain {

	#define PARTICLE_TYPE_LAUNCHER 0.0f
	#define PARTICLE_TYPE_SHELL 1.0f
		//����ٶ�
    #define MAX_VELOC glm::vec3(0.0,2.0,0.0)
		//��С�ٶ�
    #define MIN_VELOC glm::vec3(0.0,1.0,0.0)
    //�����С�ٶȲ��
    #define DEL_VELOC glm::vec3(0.0,1.0,0.0)
		//���������������
    #define MAX_LAUNCH 1.0f*1000.0f
		//����������С����
    #define MIN_LAUNCH 0.1f*1000.0f
		//��ʼ�㾫���С
    #define INIT_SIZE 30.0f
	#define MAX_SIZE 8.0f
	#define MIN_SIZE 3.0f

	const float ANGLE = 120.0f;
    const int MAX_PARTICLES = 18000;//�������ӷ���ϵͳ����������

	//��ʼ��������������
    const int INIT_PARTICLES = 10000;
	//��ѩ����
	const glm::vec3 center(0.0f);
    const float radius = 0.3f;

	struct WaterParticle
	{
		float type;
		glm::vec3 position;
		glm::vec3 velocity;
		float lifetimeMills;//����
		float size;//���ӵ㾫���С
	};

	class Fountain
	{
	public:
		Fountain();
		~Fountain();
        void initialize();
		void Render(float frametimeMills, glm::mat4& worldMatrix, glm::mat4 viewMatrix, glm::mat4& projectMatrix);
	private:
		bool InitFountain();
		void UpdateParticles(float frametimeMills);//�������ӵ�λ�õ�
		void InitRandomTexture(unsigned int size);//����1ά�������
		void RenderParticles(glm::mat4& worldMatrix, glm::mat4& viewMatrix, glm::mat4& projectMatrix);
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
