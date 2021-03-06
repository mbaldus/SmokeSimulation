#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PARTICLES 5000

#define NEIGHBOURS 0
#define DENSITY 1
#define SPH 2
#define INTEGRATION 3


#include <myCL/clSphOld.h>
#include <Util/util.h>
#include <GL/GLTools.h>
#include <GL/CVK_Trackball.h>
#include <GL/ShaderProgram.h>
#include <GL/CVK_Sphere.h>
 


int main(void) {
	printf("OpenCL Particles\n");
	
	GLFWwindow* window = GLTools::generateWindow(1280,720,100,100,"SPH Demo");

	Trackball trackball(GLTools::getWidth(window),GLTools::getHeight(window));
	Sphere* sphere = new Sphere(0.25);
	
	CLsphOld* sph = new CLsphOld();
    
	std::string kernelFile = loadfromfile(KERNELS_PATH "/SPHwater.cl");
    sph->loadProgram(kernelFile);

	//initialize our particle system with positions, velocities and color
	int num = NUM_PARTICLES;
	std::vector<glm::vec4> pos(num);
	std::vector<glm::vec4> vel(num);
	std::vector<int> neighbours(num*50);
	std::vector<int> counter(num); //not num (count of neighbours)
	std::vector<float> density(num);
	std::vector<float> pressure(num);
	std::vector<float> viscosity(num);
	std::vector<float> mass(num);
	std::vector<glm::vec4> forceIntern(num);
	
	//spawn on plane
	for (int i = 0; i <num; i++)
	{
		/*float x = rand_float(-0.25,0.25);
		float z = rand_float(-0.25,0.25);
		float y = rand_float(-0.25,0.25);*/

		float x = rand_float(-0.49,-0.25);
		float z = rand_float(0,0.25);
		float y = rand_float(-0.49,-0.25);

		pos[i] = glm::vec4(x,y,z,1.0f);
		//printf("pos: %f,%f,%f\n", pos[i].x, pos[i].y, pos[i].z);
		
		//float life_r =rand_float(0.0f,1.0f);
		//float rand_y = rand_float(-1.0,3);
		//glm::vec3 initial_vel =  glm::vec3(x,rand_y,z);
		
		//printf("life: %f\n", life_r);
		//vel[i] = glm::vec4(initial_vel, life_r);
		
		vel[i] = glm::vec4(0.4,0,0,0);
		
		//printf("vel: %f,%f,%f\n", vel[i].x, vel[i].y, vel[i].z);
		density[i] = 0.0f;
		pressure[i] = 1.0f;
		viscosity[i] = 1.0f;
		mass[i] = 0.25f;
		forceIntern[i] = glm::vec4(0,0,0,0);
		counter[i]=0;
	}
	
	sph->loadData(pos,vel,neighbours,counter,density,pressure,viscosity,mass,forceIntern); 
	sph->genNeighboursKernel();
	sph->genDensityPressureKernel();
	sph->genIntegrationKernel();
	sph->genSPHKernel();

	//###################################################################
	//				GL ShaderProgram and Camera Settings

	ShaderProgram* shaderprogram = new ShaderProgram("/simpleVS.vert", "/simpleFS.frag");
	//ShaderProgram* shaderprogram = new ShaderProgram("/simpleVS.vert", "/pointSpheres.frag");

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f,-0.1f,1.0f),glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 projection = glm::perspective(45.f, GLTools::getRatio(window), 0.1f, 100.f);
	shaderprogram->update("model",model);
	shaderprogram->update("view",view);
	shaderprogram->update("projection",projection);
	shaderprogram->update("lightDir", glm::vec3(0,0,1)); // for pointSpheres.frag
	//###################################################################

	std::function<void(double)> loop = 
		[&sph,
		&shaderprogram,
		&trackball, &sphere,
		&view, 
		&neighbours,
		&window](double deltatime)
	{
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
		shaderprogram->use();
		trackball.update(window,view);
		shaderprogram->update("view", view);
		
		sph->runKernel(NEIGHBOURS);  //0 == Nachbarschaftssuche
		sph->runKernel(DENSITY);	 //1 == Dichte und Druckberechnung
		sph->runKernel(SPH);		 //2 == Sph
		sph->runKernel(INTEGRATION); //3 == Integration

		sph->render();
	};
	
	GLTools::render(window, loop);

	//cleanup
	GLTools::destroyWindow(window);
	delete shaderprogram;
	delete sph;
	delete sphere;
   return 0;
}

