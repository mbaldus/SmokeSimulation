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

		std::vector<cl::Memory> cl_vbos; //0: position VBO, 1: color VBO
		cl::Buffer cl_neighbours;   //Neighbours
		cl::Buffer cl_velocities;	//particle velocity
		
		//SPH parameters and Buffers
		cl::Buffer cl_density;
		cl::Buffer cl_pressure;
		cl::Buffer cl_viscosity;
		cl::Buffer cl_mass;
		cl::Buffer cl_forceIntern;

		float dt;
		float poly6;
		float smoothingLength;
		
		int p_vbo; //position VBO
		int c_vbo; //color VBO
		int m_num; //nuber of particles
		size_t array_size; //the size of our arrays num * sizeof(Vec4)
		size_t int_size; //size of particles amount * count of saved neighbours per particle
		size_t float_size;

		//default constructor initializes OpenCL Context and chooses platform and device
		CLsph();
		//default deconstructor releases OpenCL objects and frees device memory
		~CLsph();

		//load OpenCL program from a file
		//pass in the kernel source code as a string. 
			
		void loadProgram(std::string kernel_source);
		

		//setup the data for the kernel
		void loadData(std::vector<glm::vec4> pos, 
					  std::vector<glm::vec4> vel,
					  std::vector<int> neighbours,
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