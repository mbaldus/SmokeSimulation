#define __CL_ENABLE_EXCEPTIONS

#include <vector>
#include <CL/cl.hpp>

class CLstart
{
	public:

		cl::Buffer cl_a;
		cl::Buffer cl_b;
		cl::Buffer cl_c;
		int m_num; //<! size of the array

		//default constructor initializes OpenCL Context and chooses platform and device
		CLstart();
		//default deconstructor releases OpenCL objects and frees device memory
		~CLstart();

		//load OpenCL program from a file
		//pass in the kernel source code as a string. 
		void loadProgram(std::string kernel_source);
		
		//setup data for the kernel
		void popCorn();
		
		//execute the kernel
		void runKernel();

	private: 

		//handles for creating an opencl context
		
		//device variables
		unsigned int m_devicesUsed;
		std::vector<cl::Device> m_devices;

		cl::Context m_context;
		cl::CommandQueue m_queue;
		cl::Program m_program;
		cl::Kernel m_kernel;
		
		//debugging variables
		cl_int m_err;
		cl::Event m_event;
};