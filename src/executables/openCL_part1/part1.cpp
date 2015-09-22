#include <stdio.h>

#include <Default/cll.h>
#include <Default/util.h>

void CL::popCorn()
{
	printf("part1: in popCorn\n");

	//initialize our kernel from the program
	try
	{
		//name of the string must be same as defined in the cl.file
		m_kernel = cl::Kernel(m_program, "part1", &m_err);
	}catch(cl::Error er)
	{
		printf("part1: Error: %s(%d)\n", er.what(), er.err()); 
	}

	//initialize CPU memory arrays, send them to the device and set the kernel arguments
	float *a = new float[m_num];
	float *b = new float[m_num];
	float *c = new float[m_num];

	for(int i=0; i < m_num; i++)
    {
        a[i] = 1.0f * i;
        b[i] = 1.0f * i;
        c[i] = 0.0f;
    }

	printf("part1: Creating OpenCL Arrays\n");
	size_t array_size = sizeof(float) * m_num;

	//input arrays
	cl_a = cl::Buffer(m_context, CL_MEM_READ_ONLY, array_size, NULL, &m_err);
	cl_b = cl::Buffer(m_context, CL_MEM_READ_ONLY, array_size, NULL, &m_err);
	//output array
	cl_c = cl::Buffer(m_context, CL_MEM_WRITE_ONLY, array_size, NULL, &m_err);

	printf("part1: Pushing data to the GPU\n");
	m_err = m_queue.enqueueWriteBuffer(cl_a, CL_TRUE, 0, array_size, a, NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_b, CL_TRUE, 0, array_size, b, NULL, &m_event);
	m_err = m_queue.enqueueWriteBuffer(cl_c, CL_TRUE, 0, array_size, c, NULL, &m_event);

	//set the arguments for the kernel (__kernel void part1(__global float *a, __global float *b, __global float *c))
	m_err = m_kernel.setArg(0,cl_a);
	m_err = m_kernel.setArg(1,cl_b);
	m_err = m_kernel.setArg(2,cl_c);

	//wait for the command queue to finish commands before proceeding
	m_queue.finish();

	delete [] a;
	delete [] b;
	delete [] c;
	
	printf("part1: out: popCorn()\n######################################################\n");
}

void CL::runKernel()
{
	printf("part1: in: runKernel()\n");

	//execute the kernel
	//cl::NDRange(m_num) is the global workgroup size (one dimensional as arrays)
	m_err = m_queue.enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(m_num), cl::NullRange, NULL, &m_event);
	printf("part1: clEnqueueNDRangeKernel: %s\n", oclErrorString(m_err));
	m_queue.finish();

	//check calculation by reading form the device memory and print out the result
	float *c_done = new float [m_num];
	m_err = m_queue.enqueueReadBuffer(cl_c, CL_TRUE, 0, sizeof(float) * m_num, c_done, NULL, &m_event);
	printf("part1: clEnqueueReadBuffer: %s \n", oclErrorString(m_err));

	for(int i =0 ; i < m_num; i++)
	{
		printf("part1: c_done[%d] = %g \n", i, c_done[i]);
	}
	printf("part1: out: runKernel()\n######################################################\n");
}

	 