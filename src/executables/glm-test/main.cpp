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

	std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if(all_platforms.size()==0){
        std::cout<<" No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    cl::Platform default_platform=all_platforms[0];
    std::cout << "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\n";

    return 0;
}