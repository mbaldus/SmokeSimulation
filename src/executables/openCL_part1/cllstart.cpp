#include <stdio.h>
#include <iostream>
#include <string>

#include "cllstart.h"
#include <Util/util.h>

CLstart::CLstart()
{
	printf("cll.cpp: in: Constructor\n");
	m_num = 10000000;
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
			{CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[1])(), 0};
	
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

CL::~CL()
{
}

void CLstart::loadProgram(std::string kernel_source)
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
	}catch(cl::Error er)
	{
		printf("cll.cpp: Error: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}

	printf("cll.cpp: build program\n");

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

void CLstart::popCorn()
{
	printf("part1: in popCorn\n");

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_kernel = cl::Kernel(m_program, "part1", &m_err);
	}catch(cl::Error er)
	{
		printf("part1: Error: %s(%d)\n", er.what(), er.err()); 
	}

	//initialize CPU memory arrays, send them to the device and set the kernel arguments
	float *a = new float[m_num];
	float *b = new float[m_num];
	float *c = new float[m_num];

	for(int i=0; i < m_num; i++)
    {
        a[i] = 1.0f * i;
        b[i] = 1.0f * i;
        c[i] = 0.0f;
    }

	printf("part1: Creating OpenCL Arrays\n");
	size_t array_size = sizeof(float) * m_num;

	//input arrays
	cl_a = cl::Buffer(m_context, CL_MEM_READ_ONLY, array_size, NULL, &m_err);
	cl_b = cl::Buffer(m_context, CL_MEM_READ_ONLY, array_size, NULL, &m_err);
	//output array
	cl_c = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, array_size, NULL, &m_err);

	printf("part1: Pushing data to the GPU\n");
	m_err = m_queue.enqueueWriteBuffer(cl_a, CL_TRUE, 0, array_size, a, NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_b, CL_TRUE, 0, array_size, b, NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_c, CL_TRUE, 0, array_size, c, NULL, &m_event);

	//set the arguments for the kernel (__kernel void part1(__global float *a, __global float *b, __global float *c))
	m_err = m_kernel.setArg(0,cl_a);
	m_err = m_kernel.setArg(1,cl_b);
	m_err = m_kernel.setArg(2,cl_c);

	//wait for the command queue to finish commands before proceeding
	m_queue.finish();

	delete [] a;
	delete [] b;
	delete [] c;
	
	printf("part1: out: popCorn()\n######################################################\n");
}

void CLstart::runKernel()
{
	printf("part1: in: runKernel()\n");

	//execute the kernel
	//cl::NDRange(m_num) is the global workgroup size (one dimensional as arrays)
	m_err = m_queue.enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(m_num), cl::NullRange, NULL, &m_event);
	printf("part1: clEnqueueNDRangeKernel: %s\n", oclErrorString(m_err));
	m_queue.finish();

	//check calculation by reading form the device memory and print out the result
	float *c_done = new float [m_num];
	m_err = m_queue.enqueueReadBuffer(cl_c, CL_TRUE, 0, sizeof(float) * m_num, c_done, NULL, &m_event);
	printf("part1: clEnqueueReadBuffer: %s \n", oclErrorString(m_err));

	printf("RESULT c_done[%d] = %g \n", m_num-1, c_done[m_num-1]);
	/*for(int i =0 ; i < m_num; i++)
	{
		printf("part1: c_done[%d] = %g \n", i, c_done[i]);
	}*/
	printf("part1: out: runKernel()\n######################################################\n");
}
	 

