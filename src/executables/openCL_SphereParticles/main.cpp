#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PARTICLES 10000

#include <myCL/clParticles.h>
#include <Util/util.h>
#include <GL/GLTools.h>
#include <GL/CVK_Trackball.h>
#include <GL/ShaderProgram.h>
#include <GL/CVK_Sphere.h>
 
int main(void) {
	printf("OpenCL Particles\n");
	
	GLFWwindow* window = GLTools::generateWindow(1280,720,100,100,"SphereSpawn Demo");

	Trackball trackball(GLTools::getWidth(window),GLTools::getHeight(window));
	Sphere* sphere = new Sphere(0.25);
	
	CLparticles* example = new CLparticles();

	double xpos,ypos;
    
	std::string kernel_source = loadfromfile(KERNELS_PATH "/sphereCollision.cl");
    example->loadProgram(kernel_source);
	
	//initialize our particle system with positions, velocities and color
	int num = NUM_PARTICLES;
	std::vector<glm::vec4> pos(num);
	std::vector<glm::vec4> vel(num);

	//spawn on sphere
	//fill vectors with initial data
	//float radius = 0.31f;
	//for (int i = 0; i <num; i++)
	//{
	//	float thetha = rand_float(0,3.14f);
	//	float phi = rand_float(0, 2*3.14f);
	//	
	//	float x = radius * cos(phi)*sin(thetha);
	//	float y = radius * cos(thetha);
	//	float z = radius * sin(thetha)*sin(phi);

	//	pos[i] = glm::vec4(x,y,z,1.0f);

	//	//printf("pos: %f,%f,%f\n", pos[i].x, pos[i].y, pos[i].z);
	//	
	//	float life_r =rand_float(0.0f,1.0f);
	//	float rand_y = rand_float(-1.0,3);
	//	glm::vec3 initial_vel =  glm::vec3(x*3,y*3,z*3);
	//	
	//	//printf("life: %f\n", life_r);
	//	vel[i] = glm::vec4(initial_vel, life_r);
	//	//vel[i] = glm::vec4(0,0,0, life_r);
	//	//printf("vel: %f,%f,%f\n", vel[i].x, vel[i].y, vel[i].z);
	//}


	//spawn on plane
	for (int i = 0; i <num; i++)
	{
		float x = rand_float(-0.1,0.1);
		float z = rand_float(0.0,0.1);
		float y = -0.5;
		pos[i] = glm::vec4(x,y,z,1.0f);
		//printf("pos: %f,%f,%f\n", pos[i].x, pos[i].y, pos[i].z);
		
		float life_r =rand_float(0.0f,1.0f);
		float rand_y = rand_float(-1.0,3);
		glm::vec3 initial_vel =  glm::vec3(x,rand_y,z);
		
		//printf("life: %f\n", life_r);
		vel[i] = glm::vec4(initial_vel, life_r);
		
		//printf("vel: %f,%f,%f\n", vel[i].x, vel[i].y, vel[i].z);
	}

	example->loadData(pos,vel); 
	example->genKernel();

	//###################################################################
	//				GL ShaderProgram and Camera Settings

	ShaderProgram* shaderprogram = new ShaderProgram("/simpleVS.vert", "/sphereFS.frag");
	
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f,-0.1f,1.0f),glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 projection = glm::perspective(45.f, GLTools::getRatio(window), 0.1f, 100.f);
	shaderprogram->update("model",model);
	shaderprogram->update("view",view);
	shaderprogram->update("projection",projection);
	//###################################################################

	//reverse gravity
	int reverse = 0;
	bool sphereColor = true;

	std::function<void(double)> loop = 
		[&example,
		&shaderprogram,
		&trackball, &sphere,
		&view, 
		&xpos, &ypos, &reverse, &sphereColor,
		&window](double deltatime)
	{
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		shaderprogram->use();
		trackball.update(window,view);
		shaderprogram->update("view", view);

		//some mousebuttonfun that reverses the gravity
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			reverse = 1;
		}else {
			reverse = 0;
		}
		
		//render sphere in grey
		sphereColor=true;
		shaderprogram->update("sphereColor", sphereColor);
		sphere->render();	
		sphereColor=false;
		shaderprogram->update("sphereColor", sphereColor);

		example->runKernel(reverse);
		example->render();
		
		
	};
	
	GLTools::render(window, loop);

	//cleanup
	GLTools::destroyWindow(window);
	delete shaderprogram;
	delete example;
	delete sphere;
   return 0;
}
