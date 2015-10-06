#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <iostream>

#include <Part1/cll.h>
#include <Util/util.h>
 
int main(void) {
	printf("Main: Hello OpenCL\n");
	double time_spent;
	clock_t begin;
	begin = clock();

	std::cout << "start..." << std::endl;

	CL example;
	std::string kernel_source = loadfromfile(KERNELS_PATH "/part1.cl");
    //load and build our CL program from the file
	example.loadProgram(kernel_source);
	example.popCorn();
	example.runKernel();
	

	time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
	std::cout << "Simulation time: " << time_spent << " s" << std::endl;

   return 0;
}