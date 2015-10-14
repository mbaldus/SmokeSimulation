#ifndef CL2_H
#define CL2_H

#define __CL_ENABLE_EXCEPTIONS



#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>

#include <CL/cl.hpp>

#include <GL/buffer.h>
#include <GL/glcorearb.h>
#include <Util/util.h>

#include <glm/glm.hpp>

class CLparticles
{
	public:

		std::vector<cl::Memory> cl_vbos; //0: position VBO, 1: color VBO
		cl::Buffer cl_velocities;	//particle velocity
		cl::Buffer cl_pos_gen;		//want to have the start points reseting particles
		cl::Buffer cl_vel_gen;		//want to have the start velocities for reseting particles
		
		int p_vbo; //position VBO
		int c_vbo; //color VBO
		int m_num; //nuber of particles
		size_t array_size; //the size of our arrays num * sizeof(Vec4)

		//default constructor initializes OpenCL Context and chooses platform and device
		CLparticles();
		//default deconstructor releases OpenCL objects and frees device memory
		~CLparticles();

		//load OpenCL program from a file
		//pass in the kernel source code as a string. 
		void loadProgram(std::string kernel_source);

		//setup the data for the kernel
		void loadData(std::vector<glm::vec4> pos, std::vector<glm::vec4> vel, std::vector<glm::vec4> color);
		
		//setup data for the kernel
		void genKernel();
		
		//execute the kernel
		void runKernel();

		void render();

	private: 

		//handles for creating an opencl context
		
		//device variables
		unsigned int m_devicesUsed;
		std::vector<cl::Device> m_devices;

		cl::Context m_context;
		cl::CommandQueue m_queue;
		cl::Program m_program;
		cl::Kernel m_kernel;
		
		//debugging variables
		cl_int m_err;
		cl::Event m_event;
};

#endif //CL2_H