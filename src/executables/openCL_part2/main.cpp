#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#define NUM_PARTICLES 1000000

#include <Part2/cll2.h>
#include <Util/util.h>
#include <GL/GLTools.h>
#include <GL/CVK_Trackball.h>
 
float rand_float(float mn, float mx)
{
    float r = rand() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
}

int main(void) {
	printf("Main: Hello OpenCL\n");
	
	GLFWwindow* window = GLTools::generateWindow(1280,720,100,100,"Part2 Demo");

	Trackball trackball(GLTools::getWidth(window),GLTools::getHeight(window));
	
	CL2* example = new CL2();

	double xpos,ypos;

	//load kernel.cl file 
	std::string kernel = loadfromfile(KERNELS_PATH "/kernel.cl");
    example->loadProgram(kernel);
	
	//initialize our particle system with positions, velocities and color
	int num = NUM_PARTICLES;
	std::vector<glm::vec4> pos(num);
	std::vector<glm::vec4> vel(num);
	std::vector<glm::vec4> color(num);

	//fill vectors with initial data
	for (int i = 0; i <num; i++)
	{
		//distribute the particles in a random circle around z axis
		float rad = rand_float(0.1,0.5);
		float x = rad*sin(2*3.14*i/num);
		float z = 0;
		//float z = rad;
	//	float y = -1;
		float y = rad*cos(2*3.14*i/num);
		pos[i] = glm::vec4(x,y,z,1.0f);
		//printf("pos: %f,%f,%f\n", pos[i].x, pos[i].y, pos[i].z);
		
		float life_r =rand_float(0.0f,1.0f);
		//printf("life: %f\n", life_r);
		vel[i] = glm::vec4(0.0,0.0f,3.0f, life_r);
		//printf("vel: %f,%f,%f\n", vel[i].x, vel[i].y, vel[i].z);

		//just make them red and full alpha
        color[i] = glm::vec4(1, 1, 1, 1.0f);
	}
	example->loadData(pos,vel,color);
	example->genKernel();

	std::function<void(double)> loop = 
		[&example,
		&trackball,
		&xpos, &ypos,
		&window](double deltatime)
	{
		example->render();
	//	trackball.update(window);
		/*glfwGetCursorPos(window,&xpos,&ypos);
		std::cout << xpos << std::endl;*/
	};
	
	GLTools::render(window, loop);

	//cleanup
	GLTools::destroyWindow(window);
	delete example;
   return 0;
}

