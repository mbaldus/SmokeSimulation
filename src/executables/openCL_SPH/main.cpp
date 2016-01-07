#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PARTICLES 10000

#define NEIGHBOURS 0
#define DENSITY 1
#define SPH 2
#define INTEGRATION 3


#include <myCL/clSph.h>
#include <Util/util.h>
#include <GL/GLTools.h>
#include <GL/CVK_Trackball.h>
#include <GL/ShaderProgram.h>
#include <GL/CVK_Sphere.h>
#include <GL/Texture.h>
 

void loadAndBindTextures(Texture* textures[], ShaderProgram* shaderprogram)
{
	textures[0] = new Texture(TEXTURES_PATH "/smoke10.png");
	textures[1] = new Texture(TEXTURES_PATH "/smoke1.png");
	textures[2] = new Texture(TEXTURES_PATH "/smoke2.png");
	textures[3] = new Texture(TEXTURES_PATH "/smoke3.png");
	textures[4] = new Texture(TEXTURES_PATH "/smoke4.png");
	textures[5] = new Texture(TEXTURES_PATH "/smoke5.png");
	textures[6] = new Texture(TEXTURES_PATH "/smoke6.png");
	textures[7] = new Texture(TEXTURES_PATH "/smoke7.png");
	textures[8] = new Texture(TEXTURES_PATH "/smoke8.png");
	textures[9] = new Texture(TEXTURES_PATH "/smoke9.png");
	
	shaderprogram->use();

	for (int i = 0; i < 10; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, textures[i]->getHandle());
	}
}

int main(void) {
	printf("OpenCL Particles\n");
	
	GLFWwindow* window = GLTools::generateWindow(1280,720,100,100,"SPH Demo");
	glClearColor(0.85, 0.85, 0.85, 0.0);

	Trackball trackball(GLTools::getWidth(window),GLTools::getHeight(window));
	Sphere* sphere = new Sphere(0.25);

	//actual best result: mass = 0.000025f
	CLsph* sph = new CLsph(0.00375f,0.05f,0.59);
    
	std::string kernel_source = loadfromfile(KERNELS_PATH "/SPH.cl");
    sph->loadProgram(kernel_source);

	//initialize our particle system with positions, velocities and color
	int num = NUM_PARTICLES;
	std::vector<glm::vec4> pos(num);
	std::vector<glm::vec4> vel(num);
	std::vector<float> life(num);
	std::vector<float> rndmSprite(num);
	std::vector<int> neighbours(num*50);
	std::vector<int> counter(num); //not num (count of neighbours)
	std::vector<float> density(num);
	std::vector<float> pressure(num);
	std::vector<float> viscosity(num);
	std::vector<float> mass(num);
	std::vector<glm::vec4> forceIntern(num);
	
	for (int i = 0; i <num; i++)
	{

		float x,y,z, life_r,rand_vel;
		//####################################
		//	   type 1 (from side)
		//b = 1.5 , life -= 0.25
	/*	 x = rand_float(-0.75,-0.5);
		 z = rand_float(-0.125,0.125);
		 y = rand_float(0.25,0.5);
		
		 rand_vel = rand_float(0.5,3);
		vel[i] = glm::vec4(rand_vel,0.5*rand_vel,0,0);*/
		
		
		//####################################
		//     type 2 (from chimney)
		//b = 50, life -= 0,15
	
		 x = rand_float(-0.125,0.125);
		 z = rand_float(-0.125,0.125);
		y = rand_float(0.125,0.25);

		rand_vel = rand_float(0.3,2.5);
		float rand_xz = rand_float(-0.2,0.2);
		vel[i] = glm::vec4(rand_xz,rand_vel,rand_xz,0);
		
		
		//####################################
		//	  type 3 two sources (chimney)
		//b = 50, life -= 0,25

	/*	if(i % 2 == 0)
		{
		 x = rand_float(-0.125,0.125);
		 z = rand_float(-0.125,0.125);
		 y = rand_float(-0.49,-0.25);

		 rand_vel = rand_float(1.0,2.75);
		 vel[i] = glm::vec4(0,rand_vel,0,0);
		}
		else if (i % 2 == 1)
		{
		 x = rand_float(-0.125,0.125);
		 z = rand_float(-0.125,0.125);
		 y = rand_float(1.99,1.75);

		 rand_vel = rand_float(1.5,2.5);
		 vel[i] = glm::vec4(0,-rand_vel,0,0);
		}
*/
		//####################################
		//     type 4 two sources (side)
		// buoyancy 1.5 ? , life -0.25
	/*	if(i % 2 == 0)
		{
		 x = rand_float(-0.9,-0.75);
		 z = rand_float(-0.25,0.25);
		 y = rand_float(-0.40,-0.10);
		 
		 float rand_z = rand_float(-0.2,0.2);
		 rand_vel = rand_float(0.7,3.5);
		 vel[i] = glm::vec4(1.5*rand_vel,0.5*rand_vel,rand_z,0);
		}
		else if (i % 2 == 1)
		{
		 x = rand_float(0.70,0.9);
		 z = rand_float(-0.25,0.25);
		 y = rand_float(-0.40,-0.10);
		 
		 float rand_z = rand_float(0.0,0.3);
		 rand_vel = rand_float(0.7,4.5);
		 vel[i] = glm::vec4(-1.5*rand_vel,1*rand_vel,rand_z,0);
		}*/

		pos[i] = glm::vec4(x,y,z,1.0f);
		life[i] = rand_float(0.0f,1.0f);
		rndmSprite[i] = float(i % 10);
		density[i] = 0.0f;
		pressure[i] = 1.0f;
		viscosity[i] = 1.0f;
		mass[i] = 0.000025f;
		forceIntern[i] = glm::vec4(0,0,0,0);
		counter[i]=0;

	}
	
	sph->loadData(pos,vel,life,rndmSprite, neighbours,counter,density,pressure,viscosity,mass,forceIntern); 
	sph->genNeighboursKernel();
	sph->genDensityKernel();
	sph->genIntegrationKernel();
	sph->genSPHKernel();

	//###################################################################
	//				GL ShaderProgram and Camera Settings
	ShaderProgram* shaderprogram = new ShaderProgram("/smoke.vert", "/smokeSprite.frag");

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f,-0.1f,1.0f),glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 projection = glm::perspective(45.f, GLTools::getRatio(window), 0.1f, 100.f);
	shaderprogram->update("model",model);
	shaderprogram->update("view",view);
	shaderprogram->update("projection",projection);
	shaderprogram->update("lightDir", glm::vec3(0,0,1)); // for pointSpheres.frag

	//						    Textures
	Texture* textures[10];
	loadAndBindTextures(textures, shaderprogram);
	GLint smokeTexs[10]={0,1,2,3,4,5,6,7,8,9}; //Array that Contains all Smoketextures for Uniform
	glUniform1iv(glGetUniformLocation(shaderprogram->getShaderProgramHandle(), "smokeTextures"), 10, smokeTexs);
	
	//###################################################################
	int delay = 0;
	int i = 0;
	std::function<void(double)> loop = 
		[&sph,
		&shaderprogram,
		&trackball, &sphere,
		&model,
		&view, &delay, &i,
		&neighbours,
		&window](double deltatime)
	{
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.005);*/

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		shaderprogram->use();
		trackball.update(window,view);
		shaderprogram->update("view", view);

		if (delay % 60 == 0)
		{
			printf("seconds = %d \n", i);
			i++;
			delay = 0;
		}
		delay++;
		
		sph->runKernel(NEIGHBOURS);  //0 == Nachbarschaftssuche
		sph->runKernel(DENSITY);	 //1 == Dichte und Druckberechnung
		sph->runKernel(SPH);		 //2 == Sph
		sph->runKernel(INTEGRATION); //3 == Integration

		sph->render();

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

	};
	
	GLTools::render(window, loop);

	//cleanup
	GLTools::destroyWindow(window);
	delete shaderprogram;
	delete sph;
	delete sphere;
   return 0;
}

