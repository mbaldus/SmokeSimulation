#define PI 3.14159265359

#include "clSph.h"

CLsph::CLsph(float delta, float radius, float r0, int num)
{
	printf("Initialize OpenCL Object and context \n");

	//normal sph
	m_num = num;
	dt = delta; 
	smoothingLength = radius; 
	rho0 = r0;

	poly6 = 315/(64*PI*pow(smoothingLength,9));
	spiky = -45/(PI*pow(smoothingLength,6));
	visConst = 45/(PI*pow(smoothingLength,6));

	buoyancy = 1.0f;
	lifeDeduction = 0.25;

	printf("Constants: \n dt = %f \n smoothingLength = %f \n poly6 = %f \n spiky = %f \n visConst = %f \n", 
		   dt, smoothingLength, poly6, spiky, visConst);
	printf("Number of Particles = %d \n", m_num);

	pos.resize(m_num);
	vel.resize(m_num);
	life.resize(m_num);
	rndmSprite.resize(m_num);
	isAlive.resize(m_num);
	aliveHelper.resize(m_num);
	neighbours.resize(m_num*50);
	counter.resize(m_num);
	density.resize(m_num);
	pressure.resize(m_num);
	viscosity.resize(m_num);
	mass.resize(m_num);
	forceIntern.resize(m_num);

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

CLsph::~CLsph()
{
}

void CLsph::loadProgram(std::string kernel_source)
{
	//Program Setup
	int program_length;

	printf("LOAD SPH PROGRAM \n");
	program_length = kernel_source.size();
	printf("kernel size %d\n" ,program_length);

	try
	{
		cl::Program::Sources source (1,  std::make_pair(kernel_source.c_str(), program_length));
		m_program = cl::Program(m_context, source);
		printf("build sph program\n");
	}catch(cl::Error er)
	{
		printf("Error: %s(%s)\n", er.what(), oclErrorString(er.err()));
	}

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

void CLsph::init(int mode)
{
	if(mode < 1 || mode > 4)
		mode = 1;

	float x,y,z;
	float rand_x,rand_y,rand_z;

	switch (mode)
	{
	case 1: //chimney
		for (int i = 0; i <m_num; i++)
		{
			x = rand_float(-0.125,0.125);
			z = rand_float(-0.125,0.125);
			y = rand_float(0.125,0.25);
			pos[i] = glm::vec4(x,y,z,1.0f);

			rand_x = rand_float(-0.2,0.2);
			rand_y = rand_float(0.3,2.5);
			rand_z = rand_float(-0.2,0.2);
			vel[i] = glm::vec4(rand_x,rand_y,rand_z,0);
		
			setBuoyancy(50.0f);
			setLifeDeduction(0.15);
		}
		break;

	case 2: // big chimney
		for (int i = 0; i <m_num; i++)
		{
			x = rand_float(-0.225,0.225);
			z = rand_float(-0.225,0.225);
			y = rand_float(0,0.25);
			pos[i] = glm::vec4(x,y,z,1.0f);

			rand_x = rand_float(-0.2,0.2);
			rand_y = rand_float(0.3,2.5);
			rand_z = rand_float(-0.2,0.2);
			vel[i] = glm::vec4(rand_x,rand_y,rand_z,0);
		
			setBuoyancy(50.0f);
			setLifeDeduction(0.25);
		}
		break;

	case 3: //side
		for (int i = 0; i <m_num; i++)
		{
			x = rand_float(-0.75,-0.5);
			z = rand_float(-0.125,0.125);
			y = rand_float(0.25,0.5);
			pos[i] = glm::vec4(x,y,z,1.0f);

			rand_x = rand_float(0.5,3);
			rand_y = rand_float(0.25,1.5);
			vel[i] = glm::vec4(rand_x,rand_y,0,0);
		
			setBuoyancy(1.5f);
			setLifeDeduction(0.25);
		}
		break;
	
	case 4: //two sources (chimney)
		for (int i = 0; i <m_num; i++)
		{

			if(i % 2 == 0)
			{
			x = rand_float(-0.125,0.125);
			z = rand_float(-0.125,0.125);
			y = rand_float(-0.49,-0.25);
			pos[i] = glm::vec4(x,y,z,1.0f);

			rand_x = rand_float(-0.2,0.2);
			rand_y = rand_float(1.0,1.75);
			rand_z = rand_float(-0.2,0.2);

			vel[i] = glm::vec4(rand_x,rand_y,rand_z,0);
			}
			else if (i % 2 == 1)
			{
			x = rand_float(-0.125,0.125);
			z = rand_float(-0.125,0.125);
			y = rand_float(1.99,1.75);
			pos[i] = glm::vec4(x,y,z,1.0f);

			rand_x = rand_float(-0.2,0.2);
			rand_y = rand_float(1.5,3.5);
			rand_z = rand_float(-0.2,0.2);
			vel[i] = glm::vec4(rand_x,-rand_y,rand_z,0);

			setBuoyancy(25.0f);
			setLifeDeduction(0.25);
			}
		}
		break;
	
	case 5: // two sources (side)
		for (int i = 0; i <m_num; i++)
		{
			if(i % 2 == 0)
			{
			x = rand_float(-0.9,-0.75);
			z = rand_float(-0.25,0.25);
			y = rand_float(-0.40,-0.10);
			pos[i] = glm::vec4(x,y,z,1.0f);
		 
			rand_x = rand_float(1.0,4.5);
			rand_y = rand_float(0.35,1.75);
			rand_z = rand_float(-0.2,0.2);
			
			vel[i] = glm::vec4(rand_x,rand_y,rand_z,0);
			}
			else if (i % 2 == 1)
			{
			x = rand_float(0.70,0.9);
			z = rand_float(-0.25,0.25);
			y = rand_float(-0.40,-0.10);
			pos[i] = glm::vec4(x,y,z,1.0f);

			rand_x = rand_float(1.1,4.5);
			rand_y = rand_float(0.7,4.5);
			rand_z = rand_float(0.0,0.3);
			vel[i] = glm::vec4(-rand_x,rand_y,rand_z,0);
		
		    setBuoyancy(1.5f);
		    setLifeDeduction(0.25);
			}
		}
		break;
	}

	for (int i = 0; i <m_num; i++)
	{
		life[i] = rand_float(0.0f,1.0f);
		rndmSprite[i] = float(i % 10);
		density[i] = 0.0f;
		pressure[i] = 1.0f;
		viscosity[i] = 1.0f;
		mass[i] = 0.000025f;
		forceIntern[i] = glm::vec4(0,0,0,0);
		counter[i]=0;
	
		//set x particles alive at the beginning to avoid explosions
		isAlive[i] = 0.0;
		aliveHelper[i] = 0;
		if(i < 50)
		{
		isAlive[i] = 1.0;
		aliveHelper[i] = 1;
		}
	}
}

void CLsph::reset()
{
	for (int i = 0; i <m_num; i++)
	{
		pos[i] = glm::vec4(0.0,0.0,0.0,1.0f);
		vel[i] = glm::vec4(0.0,0.0,0.0,0.0f);
		life[i] = 0;
		rndmSprite[i] = 0;
		density[i] = 0.0f;
		pressure[i] = 1.0f;
		viscosity[i] = 1.0f;
		mass[i] = 0.000025f;
		forceIntern[i] = glm::vec4(0,0,0,0);
		counter[i]=0;
		isAlive[i] = 0.0;
		aliveHelper[i] = 0;
	}
}

void CLsph::loadData()
{
	printf("LOAD DATA \n");
	//store number of particles and the size of bytes of our arrays
	m_num = pos.size();
	array_size = m_num * sizeof(glm::vec4);
	extended_int_size = m_num * sizeof(int) * 50;
	int_size = m_num * sizeof(int);
	float_size = m_num * sizeof(float);
	
	//create VBO's (util.cpp)
	p_vbo = createVBO(&pos[0], array_size, 4, 0); 
	life_vbo = createVBO(&life[0], float_size, 1,1);
	dens_vbo = createVBO(&density[0], float_size, 1, 2);
	rndm_vbo = createVBO(&rndmSprite[0], float_size, 1, 3);
	alive_vbo = createVBO(&isAlive[0], float_size, 1,4);
	//printf("p_vbo = %d, life_vbo = %d, rndm_vbo = %d \n" , p_vbo, life_vbo, rndm_vbo);

	//make sure OpenGL is finishedn before proceeding
	glFinish();

	//create OpenCL buffer from GL VBO
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, p_vbo, &m_err));
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, life_vbo, &m_err));
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, dens_vbo, &m_err));
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, rndm_vbo, &m_err));
	cl_vbos.push_back(cl::BufferGL(m_context, CL_MEM_READ_WRITE, alive_vbo, &m_err));

	//create OpenCL only arrays
	cl_velocities = cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);
	cl_pos_gen =  cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);
	cl_vel_gen =  cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);
	cl_life_gen = cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_neighbours = cl::Buffer(m_context, CL_MEM_READ_WRITE, extended_int_size, NULL, &m_err); 
	cl_counter = cl::Buffer(m_context, CL_MEM_READ_WRITE, int_size, NULL, &m_err); 
	cl_isAliveHelper = cl::Buffer(m_context, CL_MEM_READ_WRITE, int_size, NULL, &m_err); 
	cl_pressure =  cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_viscosity =  cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_mass =  cl::Buffer(m_context, CL_MEM_READ_WRITE, float_size, NULL, &m_err);
	cl_forceIntern =  cl::Buffer(m_context, CL_MEM_READ_WRITE, array_size, NULL, &m_err);


	printf("Pushing data to the GPU \n");
	//push CPU arrays to the GPU 
	//data is thightly packed in std::vector starting with the adress of the first element
	m_err = m_queue.enqueueWriteBuffer(cl_velocities, CL_TRUE,0, array_size, &vel[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_pos_gen, CL_TRUE,0, array_size, &pos[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_vel_gen, CL_TRUE,0, array_size, &vel[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_life_gen, CL_TRUE,0, float_size, &life[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_neighbours, CL_TRUE,0, extended_int_size, &neighbours[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_counter, CL_TRUE,0, int_size, &counter[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_isAliveHelper, CL_TRUE,0, int_size, &aliveHelper[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_pressure, CL_TRUE,0, float_size, &pressure[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_viscosity, CL_TRUE,0, float_size, &viscosity[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_mass, CL_TRUE,0, float_size, &mass[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_forceIntern, CL_TRUE,0, array_size, &forceIntern[0], NULL, &m_event);
	m_queue.finish();
	printf("######################################################\n");
}

void CLsph::updateData(std::vector<int> aliveHelper)
{
	int_size = m_num * sizeof(int);
	m_err = m_queue.enqueueWriteBuffer(cl_isAliveHelper, CL_TRUE,0, int_size, &aliveHelper[0], NULL, &m_event);
	m_queue.finish();
}

void CLsph::updateData()
{
	int_size = m_num * sizeof(int);
	array_size = m_num * sizeof(glm::vec4);
	extended_int_size = m_num * sizeof(int) * 50;
	float_size = m_num * sizeof(float);

	m_err = m_queue.enqueueWriteBuffer(cl_velocities, CL_TRUE,0, array_size, &vel[0], NULL, &m_event);
	//m_err = m_queue.enqueueWriteBuffer(cl_pos_gen, CL_TRUE,0, array_size, &pos[0], NULL, &m_event);
	//m_err = m_queue.enqueueWriteBuffer(cl_vel_gen, CL_TRUE,0, array_size, &vel[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_neighbours, CL_TRUE,0, extended_int_size, &neighbours[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_counter, CL_TRUE,0, int_size, &counter[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_isAliveHelper, CL_TRUE,0, int_size, &aliveHelper[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_pressure, CL_TRUE,0, float_size, &pressure[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_viscosity, CL_TRUE,0, float_size, &viscosity[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_mass, CL_TRUE,0, float_size, &mass[0], NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_forceIntern, CL_TRUE,0, array_size, &forceIntern[0], NULL, &m_event);
	m_queue.finish();
}

void CLsph::genNeighboursKernel()
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
		m_err = m_NeighboursKernel.setArg(5,cl_vbos[4]); //alive
	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsph::genDensityKernel()
{
	printf("genDensityKernel\n");
	

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_DensityKernel = cl::Kernel(m_program, "densityCalc", &m_err);
	}catch(cl::Error er)
	{
		printf("Error: %s(%d)\n", er.what(), er.err()); 
	}
	printf("generated densityCalculation kernel\n");
	//set the arguments of the kernel
	try
	{
		m_err = m_DensityKernel.setArg(0,cl_vbos[0]);
		m_err = m_DensityKernel.setArg(1,cl_neighbours);
		m_err = m_DensityKernel.setArg(2,cl_counter);
		m_err = m_DensityKernel.setArg(3,cl_vbos[2]); //density
		m_err = m_DensityKernel.setArg(4,cl_pressure);
		m_err = m_DensityKernel.setArg(5,cl_mass);
		m_err = m_DensityKernel.setArg(6,smoothingLength);
		m_err = m_DensityKernel.setArg(7,poly6);
		m_err = m_DensityKernel.setArg(8,rho0);
		m_err = m_DensityKernel.setArg(9,cl_vbos[4]); //alive

	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsph::genSPHKernel()
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
		m_err = m_SphKernel.setArg(4,cl_vbos[2]); //density
		m_err = m_SphKernel.setArg(5,cl_pressure);
		m_err = m_SphKernel.setArg(6,cl_viscosity);
		m_err = m_SphKernel.setArg(7,cl_mass);
		m_err = m_SphKernel.setArg(8,cl_forceIntern);
		m_err = m_SphKernel.setArg(9,smoothingLength);
		m_err = m_SphKernel.setArg(10,spiky);
		m_err = m_SphKernel.setArg(11,visConst);
		m_err = m_SphKernel.setArg(12,cl_vbos[4]); //alive

	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsph::genIntegrationKernel()
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
		m_err = m_IntegrationKernel.setArg(2,cl_pos_gen);
		m_err = m_IntegrationKernel.setArg(3,cl_vel_gen);
		m_err = m_IntegrationKernel.setArg(4,cl_life_gen);
		m_err = m_IntegrationKernel.setArg(5,cl_vbos[1]); //life
		m_err = m_IntegrationKernel.setArg(6,cl_vbos[2]); //density
		m_err = m_IntegrationKernel.setArg(7,cl_mass);
		m_err = m_IntegrationKernel.setArg(8,cl_forceIntern);
		m_err = m_IntegrationKernel.setArg(9,rho0);
		m_err = m_IntegrationKernel.setArg(10,dt);
		m_err = m_IntegrationKernel.setArg(11,cl_vbos[4]); //alive
		m_err = m_IntegrationKernel.setArg(12,cl_isAliveHelper); 
		m_err = m_IntegrationKernel.setArg(13,buoyancy);
		m_err = m_IntegrationKernel.setArg(14,lifeDeduction);

	}catch(cl::Error er)
	{
		printf("ERROR: %s\n", er.what(), oclErrorString(er.err()));
	}
	printf("done setting kernelarguments\n");
	
	//Wait for the command queue to finish these commands before proceeding
    m_queue.finish();
	printf("######################################################\n");
}

void CLsph::runKernel(int kernelnumber)
{

	//update the system by calculating new velocities and updating positions of particles
	//make sure openGL is done using VBO's
	glFinish();

	//map OpenGL buffer object for writing from OpenCL
	//this passes in the vector of VBO buffer objects
	m_err = m_queue.enqueueAcquireGLObjects(&cl_vbos, NULL, &m_event);
	m_queue.finish();

	int localKernelSize = 1024;
	//execute the kernel
	//0 == Neighbours
	if(kernelnumber == 0)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_NeighboursKernel, cl::NullRange, cl::NDRange(m_num),cl::NullRange, NULL, &m_event); // zweites Nullrange f�r local
	}
	//1 == Density
	if(kernelnumber == 1)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_DensityKernel, cl::NullRange, cl::NDRange(m_num),cl::NullRange, NULL, &m_event);
	}
	//2 == SPH
	if(kernelnumber == 2)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_SphKernel, cl::NullRange, cl::NDRange(m_num),cl::NullRange, NULL, &m_event);
	}
	//3 == Integration
	if(kernelnumber == 3)
	{
		m_err = m_queue.enqueueNDRangeKernel(m_IntegrationKernel, cl::NullRange, cl::NDRange(m_num),cl::NullRange, NULL, &m_event);
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
		//	int* new_neighbours = new int[m_num*50];
		//	m_err = m_queue.enqueueReadBuffer(cl_neighbours, CL_TRUE, 0, sizeof(int)*m_num*50, new_neighbours, NULL, &m_event);

		//	for(int i =0 ; i < 1000; i++)
		//	{
		//	printf("neighbours[%d] = %g \n", i, new_neighbours[i]);
		//	}
		//	m_queue.finish();
		//	}

		/*if(kernelnumber == 1)
		{
		float* new_density = new float[m_num];
		m_err = m_queue.enqueueReadBuffer(cl_density, CL_TRUE, 0, sizeof(float)*m_num, new_density, NULL, &m_event);

		for(int i =0 ; i < 1000; i++)
		{
		printf("new_density[%d] = %g \n", i, new_density[i]);
		}
		m_queue.finish();
		}*/

		//if(kernelnumber == 1)
		//	{
		//	float* new_pressure = new float[m_num];
		//	m_err = m_queue.enqueueReadBuffer(cl_pressure, CL_TRUE, 0, sizeof(float)*m_num, new_pressure, NULL, &m_event);

		//	for(int i =0 ; i < 1000; i++)
		//	{
		//	printf("new_pressure[%d] = %g \n", i, new_pressure[i]);
		//	}
		//	m_queue.finish();
		//	}
		//	############################################	


}

void CLsph::render()
{
	//render Particles from VBOS
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_PROGRAM_POINT_SIZE);
	
	glDrawArrays(GL_POINTS, 0, m_num);
}

void CLsph::setBuoyancy(float b)
{
	buoyancy = b;
}

void CLsph::setLifeDeduction(float f)
{
	lifeDeduction = f;
}
