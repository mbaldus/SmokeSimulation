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
		
		//SPH parameters and Buffers
		//cl::Buffer cl_density;
		cl::Buffer cl_pressure;
		cl::Buffer cl_viscosity;
		cl::Buffer cl_mass;
		cl::Buffer cl_forceIntern;
		cl::Buffer cl_rdnmSprite;
		cl::Buffer cl_isAlive;
		//cl::Buffer cl_life;

		float dt;
		float poly6;
		float spiky;
		float visConst;
		float smoothingLength;
		float rho0;
		
		int p_vbo; //position VBO
		int life_vbo; //life VBO
		int dens_vbo; //density VBO
		int rndm_vbo; //rndm Sprite
		int m_num; //number of particles

		size_t array_size; //the size of our arrays num * sizeof(Vec4)
		size_t float_size;
		size_t int_size; //(m_num)
		size_t extended_int_size; //size of particles amount * count of saved neighbours per particle

		//default constructor initializes OpenCL Context and chooses platform and device
		CLsph(float delta, float radiush, float r0);
		//default deconstructor releases OpenCL objects and frees device memory
		~CLsph();

		//load OpenCL program from a file
		//pass in the kernel source code as a string. 
			
		void loadProgram(std::string kernel_source);
		

		//setup the data for the kernel
		void loadData(std::vector<glm::vec4> pos, 
					  std::vector<glm::vec4> vel,
					  std::vector<float> life,
					  std::vector<float> rndm,
					  std::vector<int> neighbours,
					  std::vector<int> counter,
					  std::vector<int> isAlive,
					  std::vector<float> density, 
					  std::vector<float> pressure, 
					  std::vector<float> viscosity,
					  std::vector<float> mass,
					  std::vector<glm::vec4> forceIntern);

		//setup data for the kernela
		void genNeighboursKernel();
		void genDensityKernel();
		void genSPHKernel();
		void genIntegrationKernel();

		//execute the kernel
		void runKernel(int kernelnumber);

		void render();

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