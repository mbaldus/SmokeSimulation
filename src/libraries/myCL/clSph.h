#ifndef CLSPH_H
#define CLSPH_H

#define __CL_ENABLE_EXCEPTIONS

#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>

#include <CL/cl.hpp>

#include <GL/glew.h>
#include <GL/glcorearb.h>
#include <GL/buffer.h>


#include <Util/util.h>

#include <glm/glm.hpp>

class CLsph
{
	public:

		std::vector<cl::Memory> cl_vbos; //0: position VBO, 1: lifetime VBO 2: density VBO
		cl::Buffer cl_neighbours;   //Neighbours
		cl::Buffer cl_counter; 
		cl::Buffer cl_velocities;	//particle velocity
		
		cl::Buffer cl_pos_gen;		//want to have the start points reseting particles
		cl::Buffer cl_vel_gen;
		cl::Buffer cl_life_gen; 
		
		//SPH parameters and Buffers
		cl::Buffer cl_pressure;
		cl::Buffer cl_mass;
		cl::Buffer cl_forceIntern;
		cl::Buffer cl_rdnmSprite;
		cl::Buffer cl_isAliveHelper;

		float dt;
		float poly6;
		float spikyConst;
		float visConst;
		float smoothingLength;
		float rho0;
		float buoyancy;
		float lifeDeduction;
		
		int p_vbo; //position VBO
		int life_vbo; //life VBO
		int dens_vbo; //density VBO
		int rndm_vbo; //rndm Sprite
		int alive_vbo; //particle vision
		int m_numParticles; //number of particles

		std::vector<glm::vec4> pos;
		std::vector<glm::vec4> vel;
		std::vector<float> life;
		std::vector<float> rndmSprite;
		std::vector<float> isAlive;
		std::vector<int> aliveHelper;
		std::vector<int> neighbours;
		std::vector<int> counter;
		std::vector<float> density;
		std::vector<float> pressure;
		std::vector<float> mass;
		std::vector<glm::vec4> forceIntern;

		size_t array_size; //the size of our arrays num * sizeof(Vec4)
		size_t float_size;
		size_t int_size; //(m_numParticles)
		size_t extended_int_size; //size of particles amount * count of saved neighbours per particle

		CLsph(float delta, float radiush, float r0, int num);
		~CLsph();

		//load OpenCL program string
		void loadProgram(std::string kernel_source);
		
		//pusg data to GPU
		void loadData();
		
		//update data on the gpu
		void updateData();
		void updateData(std::vector<int> aliveHelper);

		//setup data for the kernel
		void genKernels();
		void genNeighboursKernel();
		void genDensityKernel();
		void genSPHKernel();
		void genIntegrationKernel();

		//execute the kernel
		void runKernel(int kernelnumber,int mousefun=0);

		//render function
		void render();

		//initialize 1 of 4 modes
		void init(int mode=1);

		//reset particles
		void reset();

		//setter
		void setBuoyancy(float f);
		void setLifeDeduction(float f);

	private: 

		//handles for creating an opencl context
		
		//device variables
		unsigned int m_devicesUsed;
		std::vector<cl::Device> m_devices;

		cl::Context m_context;
		cl::CommandQueue m_queue;
		cl::Program m_program;
		cl::Kernel m_NeighboursKernel;
		cl::Kernel m_DensityKernel;
		cl::Kernel m_SphKernel;
		cl::Kernel m_IntegrationKernel;
		
		//debugging variables
		cl_int m_err;
		cl::Event m_event;
};

#endif //CLSPH_H