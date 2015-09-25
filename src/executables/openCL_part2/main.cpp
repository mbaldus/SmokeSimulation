#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_PARTICLES 10000000


#include <Part2/cll2.h>
#include <Part1/util.h>
#include <GL/GLTools.h>
 
float rand_float(float mn, float mx)
{
    float r = rand() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
}

int main(void) {
	printf("Main: Hello OpenCL\n");

	GLFWwindow* window = GLTools::generateWindow(1280,720,100,100,"Part2 Demo");

	CL2* example = new CL2();
    
	//load and build our CL program from the file
    #include "part2.cl" //std::string kernel_source is defined in this file
    example->loadProgram(kernel_source);
	
	//initialize our particle system with positions, velocities and color
	int num = NUM_PARTICLES;
	std::vector<glm::vec4> pos(num);
	std::vector<glm::vec4> vel(num);
	std::vector<glm::vec4> color(num);

	//fill vectors with initial data
	for (int i = 0; i <num; i++)
	{
		//distribute the particles in a random circle around z axis
		float rad = rand_float(0.1,0.8);
		float x = rad*sin(2*3.14*i/num);
		float z = 0.0f;
		float y = rad*cos(2*3.14*i/num);
		pos[i] = glm::vec4(x,y,z,1.0f);
		//printf("pos: %f,%f,%f\n", pos[i].x, pos[i].y, pos[i].z);
		
		float life_r =rand_float(0.0f,1.0f);
		//printf("life: %f\n", life_r);
		vel[i] = glm::vec4(0.0,0.0f,3.0f, life_r);
		//printf("vel: %f,%f,%f\n", vel[i].x, vel[i].y, vel[i].z);

		//just make them red and full alpha
        color[i] = glm::vec4(1, 0, 0, 1.0f);
	}
	example->loadData(pos,vel,color);
	example->genKernel();

	std::function<void(double)> loop = 
		[&example](double deltatime)
	{
		example->render();
	};

	GLTools::render(window, loop);

	//cleanup
	GLTools::destroyWindow(window);
	delete example;
   return 0;
}

