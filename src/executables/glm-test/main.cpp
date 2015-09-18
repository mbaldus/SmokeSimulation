#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_WARNINGS
#include <glm/glm.hpp>
#include <iostream>
#include <CL/cl.hpp>
 
int main() {
	int test [] = {1,2,3,4,5,6,7};
	for (auto i : test)
	{
		std::cout<< i << std::endl;
	}

	cl::Platform platform;
	std::cout << "Using platform: "<<platform.getInfo<CL_PLATFORM_NAME>()<<"\n";

    return 0;
}