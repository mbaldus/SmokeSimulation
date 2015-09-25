#include <stdio.h>
#include <iostream>
#include <string>

#include "cll2.h"
#include "util.h"
#include <GL/glcorearb.h>
#include <Part1/util.h>

CL2::CL2()
{
	printf("cll.cpp: in: Constructor\n");
	printf("cll.cpp: Initialize OpenCL Object and context \n");

	std::vector<cl::Platform> platforms;
	m_err = cl::Platform::get(&platforms);
	printf("cll.cpp: cl::Platform::get(): %s\n\n", oclErrorString(m_err));
	printf("cll.cpp: Number of platforms: %d\n", platforms.size());

	if (platforms.size() == 0 )
	{
		printf("cll.cpp: Platform size 0\n");
	}

	std::cout << "cll.cpp: Platform 0: "<<platforms[0].getInfo<CL_PLATFORM_NAME>()<<"\n";
	std::cout << "cll.cpp: Platform 1: "<<platforms[1].getInfo<CL_PLATFORM_NAME>()<<"\n";
	
	m_devicesUsed = 0;
	
	//create the context with the first platform
	cl_context_properties properties[] = 
			{
				CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
				CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
				CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[1])(), 0
			};
	
	std::cout << "cll.cpp: Using Platform 1: "<<platforms[1].getInfo<CL_PLATFORM_NAME>()<<"\n\n";
	
	//set the GPU to be the device
	try{
	m_context = cl::Context(CL_DEVICE_TYPE_GPU, properties);
	}catch (cl::Error er)
	{
		printf("cll.cpp: Error: %s(%d)\n", er.what(), er.err());
	}
	
	//get Devices from Context
	m_devices = m_context.getInfo<CL_CONTEXT_DEVICES>();
	printf("cll.cpp: Number of devices: %d\n", m_devices.size());
	//print out Device Name
	std::cout << "cll.cpp: Device 0: "<<m_devices[0].getInfo<CL_DEVICE_NAME>()<<"\n";
	std::cout << "cll.cpp: Using Device 0: "<<m_devices[0].getInfo<CL_DEVICE_NAME>()<<"\n\n";
	//create command queue
	try
	{
		m_queue = cl::CommandQueue(m_context, m_devices[m_devicesUsed], 0, &m_err);
	}catch(cl::Error er)
	{
		printf("cll.cpp: Error: %s(%d)\n", er.what(), er.err());
	}
	
	printf("cll.cpp: out: Constructor: OpenCL has been initialized \n###################################################### \n");
}

CL2::~CL2()
{
}

void CL2::loadProgram(std::string kernel_source)
{
	printf("cll.cpp: in: loadProgram()\n");
	//Program Setup


	int program_length;

	printf("cll.cpp: load the program \n");
	program_length = kernel_source.size();
	printf("cll.cpp: kernel size %d\n" ,program_length);

	try
	{
		cl::Program::Sources source (1,  std::make_pair(kernel_source.c_str(), program_length));
		m_program = cl::Program(m_context, source);
		printf("cll.cpp: build program\n");
	}catch(cl::Error er)
	{
		printf("cll.cpp: Error: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}
	//printf("cll.cpp: build program\n");

	try
	{
		m_err = m_program.build(m_devices);
	}catch(cl::Error er)
	{
		printf("cll.cpp: program.build: %s\n", oclErrorString(er.err()));
	}

	printf("cll.cpp: done building program \n");
	std::cout << "cll.cpp: Build Status: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(m_devices[0]) << std::endl;
	std::cout << "cll.cpp: Build Options: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(m_devices[0]) << std::endl;
	std::cout << "cll.cpp: Build LOG: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_devices[0]) << std::endl;

	printf("cll.cpp: out: loadProgram()\n######################################################\n");

}

void CL2::loadData(std::vector<glm::vec4> pos, std::vector<glm::vec4> vel, std::vector<glm::vec4> col)
{
	//store number of particles and the size of bytes of our arrays
	m_num = pos.size();
	array_size = m_num * sizeof(glm::vec4);
	//create VBO's (util.cpp)
	p_vbo = createVBO(&pos[0], array_size, GL_ARRAY_BUFFER, GL_STATIC_DRAW); //id 1
	c_vbo = createVBO(&col[0], array_size, GL_ARRAY_BUFFER, GL_STATIC_DRAW); //id 2

	std::cout<<">>>>>>>>>p_vbo id: "<< p_vbo<< std::endl;
	std::cout<<">>>>>>>>>c_vbo id: "<< c_vbo<< std::endl;
	//make sure OpenGL is finishedn before proceeding
	glFinish();

	//create OpenCL buffer from GL VBO
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, p_vbo, &m_err));
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, c_vbo, &m_err));

	//create OpenCL only arrays
	cl_velocities = cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);
	cl_pos_gen =  cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);
	cl_vel_gen =  cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);

	printf("Pushing data to the GPU \n");
	//push CPU arrays to the GPU 
	//data is thightly packed in std::vector starting with the adress of the first element
	m_err = m_queue.enqueueWriteBuffer(cl_velocities, CL_TRUE,0, array_size, &vel[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_pos_gen, CL_TRUE,0, array_size, &pos[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_vel_gen, CL_TRUE,0, array_size, &vel[0], NULL, &m_event);
	m_queue.finish();
}

void CL2::genKernel()
{
	printf("part2: in genKernel\n");

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_kernel = cl::Kernel(m_program, "part2", &m_err);
	}catch(cl::Error er)
	{
		printf("part2: Error: %s(%d)\n", er.what(), er.err()); 
	}
	printf("part2: generated Kernel\n");
	//set the arguments of the kernel
	try
	{
		m_err = m_kernel.setArg(0,cl_vbos[0]);
		m_err = m_kernel.setArg(1,cl_vbos[1]);
		m_err = m_kernel.setArg(2,cl_velocities);
		m_err = m_kernel.setArg(3,cl_pos_gen);
		m_err = m_kernel.setArg(4,cl_vel_gen);
	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("part2: set Kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("part2: out: genKernel()\n######################################################\n");
}

void CL2::runKernel()
{

	//update the system by calculating new velocities and updating positions of particles
	//make sure openGL is done using VBO's
	glFinish();

	//map OpenGL buffer object for writing from OpenCL
	//this passes in the vector of VBO buffer objects (position and color)
	m_err = m_queue.enqueueAcquireGLObjects(&cl_vbos, NULL, &m_event);
	m_queue.finish();

	float dt = 0.003f;
	m_kernel.setArg(5, dt); //pass the timestamp
	//execute the kernel
	m_err = m_queue.enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(m_num),cl::NullRange, NULL, &m_event);
	m_queue.finish();
	
	//release the vbos so OpenGL can play with them
	m_err = m_queue.enqueueReleaseGLObjects(&cl_vbos, NULL, &m_event);
	m_queue.finish();

}

void CL2::render()
{
	glDisable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	runKernel();


	//render Particles from VBOS
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);

	glBindBuffer(GL_ARRAY_BUFFER, c_vbo);
	glColorPointer(4, GL_FLOAT, 0,0);

	glBindBuffer(GL_ARRAY_BUFFER, p_vbo);
	glVertexPointer(4, GL_FLOAT, 0, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

	//blender?
	glDisableClientState(GL_NORMAL_ARRAY);

	glDrawArrays(GL_POINTS, 0, m_num);

	glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

