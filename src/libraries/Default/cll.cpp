#include <stdio.h>
#include <iostream>
#include <string>

#include "cll.h"
#include "util.h"

CL::CL()
{
	printf("cll.cpp: in: Constructor\n");
	m_num = 10;
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

void CL::loadProgram(std::string kernel_source)
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

