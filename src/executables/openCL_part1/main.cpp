#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include <Default\cll.h>
 
int main(void) {
	printf("Main: Hello OpenCL\n");

	CL example;

    //load and build our CL program from the file
    #include "part1.cl" //const char* kernel_source is defined in here
    example.loadProgram(kernel_source);
	example.popCorn();
	example.runKernel();

   return 0;
}