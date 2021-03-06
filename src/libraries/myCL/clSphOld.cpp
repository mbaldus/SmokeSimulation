#define PI 3.14159265359

#include "clSphOld.h"

CLsphOld::CLsphOld()
{
	printf("Initialize OpenCL Object and context \n");

	//normal sph
	dt = 0.0075f; 
	smoothingLength = 0.1f; 
	rho0 = 1000;
	//visc 20, mass 0.25, gas 0.75

	poly6 = 315/(64*PI*pow(smoothingLength,9));
	spikyConst = -45/(PI*pow(smoothingLength,6));
	visConst = 45/(PI*pow(smoothingLength,6));

	printf("Constants: \n dt = %f \n smoothingLength = %f \n poly6 = %f \n spikyConst = %f \n visConst = %f \n", 
		   dt, smoothingLength, poly6, spikyConst, visConst);

	std::vector<cl::Platform> platforms;
	m_err = cl::Platform::get(&platforms);
	printf("cl::Platform::get(): %s\n\n", oclErrorString(m_err));
	printf("Number of platforms: %d\n", platforms.size());

	if (platforms.size() == 0 )
	{
		printf("Platform size 0\n");
	}

	std::cout << "Platform 0: "<<platforms[0].getInfo<CL_PLATFORM_NAME>()<<"\n";
	std::cout << "Platform 1: "<<platforms[1].getInfo<CL_PLATFORM_NAME>()<<"\n";
	
	m_devicesUsed = 0;
	
	//create the context with the first platform
	cl_context_properties properties[] = 
			{
				CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
				CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
				CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[1])(), 0
			};
	
	std::cout << "Using Platform 1: "<<platforms[1].getInfo<CL_PLATFORM_NAME>()<<"\n\n";
	
	//set the GPU to be the device
	try{
	m_context = cl::Context(CL_DEVICE_TYPE_GPU, properties);
	}catch (cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err());
	}
	
	//get Devices from Context
	m_devices = m_context.getInfo<CL_CONTEXT_DEVICES>();
	printf("Number of devices: %d\n", m_devices.size());
	//print out Device Name
	std::cout << "Device 0: "<<m_devices[0].getInfo<CL_DEVICE_NAME>()<<"\n";
	std::cout << "Using Device 0: "<<m_devices[0].getInfo<CL_DEVICE_NAME>()<<"\n\n";
	//create command queue
	try
	{
		m_queue = cl::CommandQueue(m_context, m_devices[m_devicesUsed], 0, &m_err);
	}catch(cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err());
	}
	
	printf("OpenCL has been initialized \n###################################################### \n");
}

CLsphOld::~CLsphOld()
{
}

void CLsphOld::loadProgram(std::string kernelFile)
{
	//Program Setup
	int program_length;

	printf("LOAD SPH PROGRAM \n");
	program_length = kernelFile.size();
	printf("kernel size %d\n" ,program_length);

	try
	{
		cl::Program::Sources source (1,  std::make_pair(kernelFile.c_str(), program_length));
		m_program = cl::Program(m_context, source);
		printf("build sph program\n");
	}catch(cl::Error er)
	{
		printf("Error: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}
	//printf("cll.cpp: build program\n");

	try
	{
		m_err = m_program.build(m_devices);
	}catch(cl::Error er)
	{
		printf("program.build: %s\n", oclErrorString(er.err()));
	}

	printf("done building sph program \n");
	std::cout << "Build Status: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(m_devices[0]) << std::endl;
	std::cout << "Build Options: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(m_devices[0]) << std::endl;
	std::cout << "Build LOG: " << m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_devices[0]) << std::endl;

	printf("######################################################\n");

}

void CLsphOld::loadData(std::vector<glm::vec4> pos, std::vector<glm::vec4> vel, std::vector<int> neighbours,std::vector<int> counter, std::vector<float> density, std::vector<float> pressure, std::vector<float> viscosity, std::vector<float> mass,std::vector<glm::vec4> forceIntern)
{
	printf("LOAD DATA \n");
	//store number of particles and the size of bytes of our arrays
	m_numParticles = pos.size();
	array_size = m_numParticles * sizeof(glm::vec4);
	int_size = m_numParticles * sizeof(int) * 50;
	normal_int_size = m_numParticles * sizeof(int);
	float_size = m_numParticles * sizeof(float);
	
	//create VBO's (util.cpp)
	p_vbo = createVBO(&pos[0], array_size, 4, 0); //id 1

	//make sure OpenGL is finishedn before proceeding
	glFinish();

	//create OpenCL buffer from GL VBO
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, p_vbo, &m_err));
	//cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, c_vbo, &m_err));

	//create OpenCL only arrays
	cl_velocities = cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);
	cl_neighbours = cl::Buffer(m_context, CL_MEM_READ_WRITE, int_size, NULL, &m_err); 
	cl_counter = cl::Buffer(m_context, CL_MEM_READ_WRITE, normal_int_size, NULL, &m_err); 
	cl_density =  cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_pressure =  cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_viscosity =  cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_mass =  cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_forceIntern =  cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);


	printf("Pushing data to the GPU \n");
	//push CPU arrays to the GPU 
	//data is thightly packed in std::vector starting with the adress of the first element
	m_err = m_queue.enqueueWriteBuffer(cl_velocities, CL_TRUE,0, array_size, &vel[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_neighbours, CL_TRUE,0, int_size, &neighbours[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_counter, CL_TRUE,0, normal_int_size, &counter[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_density, CL_TRUE,0, float_size, &density[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_pressure, CL_TRUE,0, float_size, &pressure[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_viscosity, CL_TRUE,0, float_size, &viscosity[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_mass, CL_TRUE,0, float_size, &mass[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_forceIntern, CL_TRUE,0, array_size, &forceIntern[0], NULL, &m_event);
	m_queue.finish();
	printf("######################################################\n");
}

void CLsphOld::genNeighboursKernel()
{
	printf("genNeighboursKernel\n");
	

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_NeighboursKernel = cl::Kernel(m_program, "neighbours", &m_err);
	}catch(cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err()); 
	}
	printf("generated neighbours kernel\n");
	//set the arguments of the kernel
	try
	{
		m_err = m_NeighboursKernel.setArg(0,cl_vbos[0]);
		m_err = m_NeighboursKernel.setArg(1,cl_neighbours);
		m_err = m_NeighboursKernel.setArg(2,cl_counter);
		m_err = m_NeighboursKernel.setArg(3,smoothingLength);
		m_err = m_NeighboursKernel.setArg(4,cl_mass);
	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsphOld::genDensityPressureKernel()
{
	printf("genDensityPressureKernel\n");
	

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_DensityPressureKernel = cl::Kernel(m_program, "densityPressureCalc", &m_err);
	}catch(cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err()); 
	}
	printf("generated densityPressureCalculation kernel\n");
	//set the arguments of the kernel
	try
	{
		m_err = m_DensityPressureKernel.setArg(0,cl_vbos[0]);
		m_err = m_DensityPressureKernel.setArg(1,cl_neighbours);
		m_err = m_DensityPressureKernel.setArg(2,cl_counter);
		m_err = m_DensityPressureKernel.setArg(3,cl_density);
		m_err = m_DensityPressureKernel.setArg(4,cl_pressure);
		m_err = m_DensityPressureKernel.setArg(5,cl_mass);
		m_err = m_DensityPressureKernel.setArg(6,smoothingLength);
		m_err = m_DensityPressureKernel.setArg(7,poly6);
		m_err = m_DensityPressureKernel.setArg(8,rho0);

	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsphOld::genSPHKernel()
{
	printf("genSPHKernel\n");
	

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_SphKernel = cl::Kernel(m_program, "SPH", &m_err);
	}catch(cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err()); 
	}
	printf("generated sph kernel\n");
	//set the arguments of the kernel
	try
	{
		m_err = m_SphKernel.setArg(0,cl_vbos[0]);
		m_err = m_SphKernel.setArg(1,cl_velocities);
		m_err = m_SphKernel.setArg(2,cl_neighbours);
		m_err = m_SphKernel.setArg(3,cl_counter);
		m_err = m_SphKernel.setArg(4,cl_density);
		m_err = m_SphKernel.setArg(5,cl_pressure);
		m_err = m_SphKernel.setArg(6,cl_viscosity);
		m_err = m_SphKernel.setArg(7,cl_mass);
		m_err = m_SphKernel.setArg(8,cl_forceIntern);
		m_err = m_SphKernel.setArg(9,smoothingLength);
		m_err = m_SphKernel.setArg(10,spikyConst);
		m_err = m_SphKernel.setArg(11,visConst);

	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsphOld::genIntegrationKernel()
{
	printf("genIntegrationKernel\n");
	

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_IntegrationKernel = cl::Kernel(m_program, "integration", &m_err);
	}catch(cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err()); 
	}
	printf("generated integration kernel\n");
	//set the arguments of the kernel
	try
	{
		m_err = m_IntegrationKernel.setArg(0,cl_vbos[0]);
		m_err = m_IntegrationKernel.setArg(1,cl_velocities);
		m_err = m_IntegrationKernel.setArg(2,cl_density);
		m_err = m_IntegrationKernel.setArg(3,cl_mass);
		m_err = m_IntegrationKernel.setArg(4,cl_forceIntern);
		m_err = m_IntegrationKernel.setArg(5,rho0);
		m_err = m_IntegrationKernel.setArg(6,dt);


	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsphOld::runKernel(int kernelnumber)
{

	//update the system by calculating new velocities and updating positions of particles
	//make sure openGL is done using VBO's
	glFinish();

	//map OpenGL buffer object for writing from OpenCL
	//this passes in the vector of VBO buffer objects (position and color)
	m_err = m_queue.enqueueAcquireGLObjects(&cl_vbos, NULL, &m_event);
	m_queue.finish();

	//execute the kernel
	//0 == Neighbours
	if(kernelnumber == 0)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_NeighboursKernel, cl::NullRange, cl::NDRange(m_numParticles),cl::NullRange, NULL, &m_event);
	}
	//1 == Density
	if(kernelnumber == 1)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_DensityPressureKernel, cl::NullRange, cl::NDRange(m_numParticles),cl::NullRange, NULL, &m_event);
	}
	//2 == SPH
	if(kernelnumber == 2)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_SphKernel, cl::NullRange, cl::NDRange(m_numParticles),cl::NullRange, NULL, &m_event);
	}
	//3 == Integration
	if(kernelnumber == 3)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_IntegrationKernel, cl::NullRange, cl::NDRange(m_numParticles),cl::NullRange, NULL, &m_event);
	}
	
	try{
	m_queue.finish();
	}catch(cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err());
	}
	
	//release the vbos so OpenGL can play with them
	m_err = m_queue.enqueueReleaseGLObjects(&cl_vbos, NULL, &m_event);
	m_queue.finish();

		//########################################
		//following code is for debugging purposes
		//
		//	if(kernelnumber == 0)
		//	{
		//	int* new_neighbours = new int[m_numParticles*50];
		//	m_err = m_queue.enqueueReadBuffer(cl_neighbours, CL_TRUE, 0, sizeof(int)*m_numParticles*50, new_neighbours, NULL, &m_event);

		//	for(int i =0 ; i < 1000; i++)
		//	{
		//	printf("neighbours[%d] = %g \n", i, new_neighbours[i]);
		//	}
		//	m_queue.finish();
		//	}

		/*if(kernelnumber == 1)
		{
		float* new_density = new float[m_numParticles];
		m_err = m_queue.enqueueReadBuffer(cl_density, CL_TRUE, 0, sizeof(float)*m_numParticles, new_density, NULL, &m_event);

		for(int i =0 ; i < 1000; i++)
		{
		printf("new_density[%d] = %g \n", i, new_density[i]);
		}
		m_queue.finish();
		}*/

		//if(kernelnumber == 1)
		//	{
		//	float* new_pressure = new float[m_numParticles];
		//	m_err = m_queue.enqueueReadBuffer(cl_pressure, CL_TRUE, 0, sizeof(float)*m_numParticles, new_pressure, NULL, &m_event);

		//	for(int i =0 ; i < 1000; i++)
		//	{
		//	printf("new_pressure[%d] = %g \n", i, new_pressure[i]);
		//	}
		//	m_queue.finish();
		//	}
		//	############################################	


}

void CLsphOld::render()
{
	//render Particles from VBOS
//	glEnable(GL_POINT_SPRITE);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(7.5);

	glBindBuffer(GL_ARRAY_BUFFER, p_vbo); //p_vbo is 0
	//glEnableVertexAttribArray(0);
	
	glDrawArrays(GL_POINTS, 0, m_numParticles);
}

