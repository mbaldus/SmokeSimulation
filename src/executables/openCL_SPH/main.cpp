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
 


int main(void) {
	printf("OpenCL Particles\n");
	
	GLFWwindow* window = GLTools::generateWindow(1280,720,100,100,"SPH Demo");
	glClearColor(0.85, 0.85, 0.85, 0.0);

	Trackball trackball(GLTools::getWidth(window),GLTools::getHeight(window));
	Sphere* sphere = new Sphere(0.25);
	Texture* tex1 = new Texture(TEXTURES_PATH "/smoke7.png");
	Texture* tex2 = new Texture(TEXTURES_PATH "/smoke9.png");
	std::cout<<tex1->getHandle() <<std::endl;
	std::cout<<tex2->getHandle() <<std::endl;

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
		//type 1
		float x = rand_float(-0.75,-0.5);
		float z = rand_float(-0.125,0.125);
		float y = rand_float(0.25,0.5);
		
		float life_r =rand_float(0.0f,0.25f);
		float rand_vel = rand_float(0.5,1);
		vel[i] = glm::vec4(3*rand_vel,1.5*rand_vel,0,0);
		

		//type 2
		/*float x = rand_float(-0.75,-0.5);
		float z = rand_float(-0.125,0.125);
		float y = rand_float(-.5,0);

		float life_r =rand_float(0.0f,0.25f);
		float rand_vel = rand_float(0.3,1);
		vel[i] = glm::vec4(2*rand_vel,rand_vel,0,0);*/


	
		pos[i] = glm::vec4(x,y,z,1.0f);
		life[i] = life_r;
		rndmSprite[i] = float(i % 2);
		//std::cout<<rndmSprite[i]<<std::endl;
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

//	ShaderProgram* shaderprogram = new ShaderProgram("/simpleVS.vert", "/simpleFS.frag");
	//ShaderProgram* shaderprogram = new ShaderProgram("/simpleVS.vert", "/pointSpheres.frag");
	ShaderProgram* shaderprogram = new ShaderProgram("/smoke.vert", "/smokeSprite.frag");

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f,-0.1f,1.0f),glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 projection = glm::perspective(45.f, GLTools::getRatio(window), 0.1f, 100.f);
	shaderprogram->update("model",model);
	shaderprogram->update("view",view);
	shaderprogram->update("projection",projection);
	shaderprogram->update("lightDir", glm::vec3(0,0,1)); // for pointSpheres.frag

	//Textures::
	shaderprogram->use();
	GLuint t1Loc =glGetUniformLocation(shaderprogram->getShaderProgramHandle(), "tex1");
	glUniform1i(t1Loc,tex1->getHandle());
	glActiveTexture(GL_TEXTURE0+1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex1->getHandle());
	
	GLuint t2Loc =glGetUniformLocation(shaderprogram->getShaderProgramHandle(), "tex2");
	glUniform1i(t2Loc, tex2->getHandle());
	glActiveTexture(GL_TEXTURE0+2);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex2->getHandle());
	
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

