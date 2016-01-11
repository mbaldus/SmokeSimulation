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

	int num = NUM_PARTICLES;
	CLsph* sph = new CLsph(0.00375f,0.05f,0.59,num);
    
	std::string kernel_source = loadfromfile(KERNELS_PATH "/SPH.cl");
    sph->loadProgram(kernel_source);

	//###################################################################
	//						SPH INITILIZATION
	  /*
	  mode 1 => chimney
	  mode 2 => side
	  mode 3 => two sources chimney
	  mode 4 => two sources side
	  */
	sph->init(1);
	sph->loadData();
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
	int framecount = 0;
	int frameoffset= 10 ;

	std::function<void(double)> loop = 
		[&sph,
		&shaderprogram,
		&trackball, &sphere,
		&model,
		&view, &delay, &framecount, &frameoffset,
		&window](double deltatime)
	{
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		shaderprogram->use();
		trackball.update(window,view);
		shaderprogram->update("view", view);
		
	/*	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			sph->reset();
			sph->init(2);
			sph->updateData();
			framecount=0;
		}*/

		if (framecount < NUM_PARTICLES)
		{
			for (int j = 0; j < frameoffset; j++)
			{
				if(framecount*frameoffset+j<NUM_PARTICLES)
				sph->aliveHelper[framecount*frameoffset+j] = 1 ;
			}
			printf("\rParticles alive: %d", framecount*frameoffset);
		}
		
		if (delay % 2 == 0)
		{
		sph->updateData(sph->aliveHelper);
		delay = 0;
		framecount++;
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

