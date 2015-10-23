#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PARTICLES 100000

#include <myCL/cll.h>
#include <Util/util.h>
#include <GL/GLTools.h>
#include <GL/CVK_Trackball.h>
#include <GL/ShaderProgram.h>
 
float rand_float(float mn, float mx)
{
    float r = rand() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
}

int main(void) {
	printf("OpenCL Particles\n");
	
	GLFWwindow* window = GLTools::generateWindow(1280,720,100,100,"Part2 Demo");

	Trackball trackball(GLTools::getWidth(window),GLTools::getHeight(window));
	
	CLparticles* example = new CLparticles();

	double xpos,ypos;
    
	std::string kernel_source = loadfromfile(KERNELS_PATH "/part2.cl");
    example->loadProgram(kernel_source);
	
	//initialize our particle system with positions, velocities and color
	int num = NUM_PARTICLES;
	std::vector<glm::vec4> pos(num);
	std::vector<glm::vec4> vel(num);

	////fill vectors with initial data
	//for (int i = 0; i <num; i++)
	//{
	//	//distribute the particles in a random circle around z axis
	//	float rad = rand_float(0.3,1);
	//	float x = rad*sin(2*3.14*i/num);
	//	float z = rad*cos(2*3.14*i/num);
	//	float y = 0;
	//	//float y = rad*cos(2*3.14*i/num);
	//	pos[i] = glm::vec4(x,y,z,1.0f);
	//	//printf("pos: %f,%f,%f\n", pos[i].x, pos[i].y, pos[i].z);
	//	
	//	float life_r =rand_float(0.0f,1.0f);
	//	float rand_y = rand_float(-1.0,3);
	//	glm::vec3 initial_vel =  glm::vec3(x,rand_y,z);
	//	
	//	//printf("life: %f\n", life_r);
	//	vel[i] = glm::vec4(initial_vel, life_r);
	//	
	//	//printf("vel: %f,%f,%f\n", vel[i].x, vel[i].y, vel[i].z);
	//}

	//spawn on sphere
	//fill vectors with initial data
	//for (int i = 0; i <num; i++)
	//{
	//	float rand = rand_float(0.0f ,1.0f);
	//	float thetha = rand_float(0,3.14f);
	//	float phi = rand_float(0, 2*3.14f);
	//	
	//	float radius = 1.0f;
	//	float x = radius * cos(phi)*sin(thetha);
	//	float y = radius * cos(thetha);
	//	float z = radius * sin(thetha)*sin(phi);

	//	pos[i] = glm::vec4(x,y,z,1.0f);

	//	printf("pos: %f,%f,%f\n", pos[i].x, pos[i].y, pos[i].z);
	//	
	//	float life_r =rand_float(0.0f,1.0f);
	//	float rand_y = rand_float(-1.0,3);
	//	glm::vec3 initial_vel =  glm::vec3(x*5,y*5,z*5);
	//	
	//	//printf("life: %f\n", life_r);
	//	vel[i] = glm::vec4(initial_vel, life_r);
	//	//vel[i] = glm::vec4(0,0,0, life_r);
	//	//printf("vel: %f,%f,%f\n", vel[i].x, vel[i].y, vel[i].z);
	//}


	//fill vectors with initial data
	//spawn on plane
	for (int i = 0; i <num; i++)
	{
		float x = rand_float(-1.0,1.0);
		float z = rand_float(-1.0,1.0);
		float y = -1;
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

	ShaderProgram* shaderprogram = new ShaderProgram("/simpleVS.vert", "/simpleFS.frag");
	
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f,-0.1f,1.0f),glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 projection = glm::perspective(45.f, GLTools::getRatio(window), 0.1f, 100.f);
	shaderprogram->update("model",model);
	shaderprogram->update("view",view);
	shaderprogram->update("projection",projection);

	//###################################################################

	//reverse gravity
	int reverse = 0;

	std::function<void(double)> loop = 
		[&example,
		&shaderprogram,
		&trackball,
		&view,
		&xpos, &ypos, &reverse,
		&window](double deltatime)
	{
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

		example->runKernel(reverse);
		example->render();
	};
	
	GLTools::render(window, loop);

	//cleanup
	GLTools::destroyWindow(window);
	delete shaderprogram;
	delete example;
   return 0;
}

