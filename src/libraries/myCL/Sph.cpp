#define PI 3.14159265359

#include "Sph.h"

SPH::SPH(float delta, float radius, float r0, int num)
{
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
}

SPH::~SPH()
{
}

void SPH::init(int mode)
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

	case 2: //side
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
	
	case 3: //two sources (chimney)
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
	
	case 4: // two sources (side)
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

void SPH::loadData()
{
	printf("LOAD DATA \n");

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

	m_vbos.push_back(p_vbo);
	m_vbos.push_back(life_vbo);
	m_vbos.push_back(dens_vbo);
	m_vbos.push_back(rndm_vbo);
	m_vbos.push_back(alive_vbo);

	printf("######################################################\n");
}

void SPH::neighboursearch()
{	
}

void SPH::densPressCalc()
{
}

void SPH::sphCalc()
{
}

void SPH::integration()
{

}

void SPH::render()
{
	//render Particles from VBOS
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_PROGRAM_POINT_SIZE);
	
	glDrawArrays(GL_POINTS, 0, m_num);
}

void SPH::setBuoyancy(float b)
{
	buoyancy = b;
}

void SPH::setLifeDeduction(float f)
{
	lifeDeduction = f;
}
