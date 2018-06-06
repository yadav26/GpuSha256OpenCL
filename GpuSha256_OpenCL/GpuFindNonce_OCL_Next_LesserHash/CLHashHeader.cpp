/*
Author : Anshul Yadav
email : yadav26@gmail.com
LinkedIn: https://www.linkedin.com/in/anshul-yadav-2289b734/
*/

#include "CLHashHeader.h"

extern OCL_INSTANCE g_oclInstance;
extern const unsigned long int GPUTHREADS ;

BOOL WINAPI consoleHandler(DWORD signal) {

	if (signal == CTRL_C_EVENT)
		printf("Ctrl-C handled\n"); // do cleanup

	ReleaseOCL(&g_oclInstance);

	exit(0);
	return TRUE;
}


/* convert the kernel file into a string */
int convertToString(const char *filename, std::string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if (f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size + 1];
		if (!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}

	std::cout << "Error: failed to open file\n:" << filename << endl;

	return FAILURE;
}


cl_int ReleaseOCL(OCL_INSTANCE* pOcl)
{
	/*Step 12: Clean the resources.*/
	cl_int status = clReleaseKernel(pOcl->kernel);		//Release kernel.
	status = clReleaseProgram(pOcl->program);			//Release the program object.
	status = clReleaseMemObject(pOcl->inputBuffer);		//Release mem object.
	status = clReleaseMemObject(pOcl->outputNonceBuffer);		//Release mem object.
	status = clReleaseMemObject(pOcl->outputBuffer);
	status = clReleaseCommandQueue(pOcl->commandQueue);	//Release  Command queue.
	status = clReleaseContext(pOcl->context);			//Release context.

	pOcl->inputBuffer = NULL;
	pOcl->outputBuffer = NULL;
	pOcl->outputNonceBuffer = NULL;

	if (pOcl->devices != NULL)
	{
		free(pOcl->devices);
		pOcl->devices = NULL;
	}

	return status;
}



cl_int InitializeOCL(OCL_INSTANCE *pOcl,
	const char *filename,
	unsigned char* input,
	unsigned char* output)
{

	//Set cleaning handle for gpu memory release on breaking in application.
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		printf("\nERROR: Could not set control handler");
		return 1;
	}

	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout << "Error: Getting platforms!" << endl;
		return FAILURE;
	}

	/*For clarity, choose the first available platform. */
	if (numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint				numDevices = 0;
	cl_device_id        *devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
	if (numDevices == 0)	//no GPU available.
	{
		cout << "No GPU device available." << endl;
		cout << "Choose CPU as default device." << endl;
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	}


	/*Step 3: Create context.*/
	pOcl->context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

	/*Step 4: Creating command queue associate with the context.*/
	pOcl->commandQueue = clCreateCommandQueue(pOcl->context, devices[0], 0, NULL);

	/*Step 5: Create program object */

	string sourceStr;
	status = convertToString(filename, sourceStr);
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };

	pOcl->program = clCreateProgramWithSource(pOcl->context, 1, &source, sourceSize, NULL);

	/*Step 6: Build program. */
	status = clBuildProgram(pOcl->program, 1, devices, NULL, NULL, NULL);
	if (status < 0)
	{
		cout << "Kernel has syntax errors, compilation failed. Err = " << status << endl;
		return status;
	}


	//size_t inStrlength = strlen(input);
	//cout << "input string : [" << input << "]" << endl;
	//int outputlength = HASH_LENGTH * GPUTHREADS;
	//size_t outputSize = outputlength * sizeof(char);

	//pOcl->inputBuffer = clCreateBuffer(pOcl->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, (inStrlength + 1) * sizeof(char), (void *)input, NULL);
	//pOcl->outputBuffer = clCreateBuffer(pOcl->context, CL_MEM_WRITE_ONLY, outputSize, NULL, NULL);


	return status;
}

cl_int RunGpu_Loads( UINT64 gpuThreads, POCL_INSTANCE pOcl,
	unsigned char* input,
	cl_uint inputSize,
	char* output,
	const char* kernel_name,
	cl_ulong startIndex,
	cl_uint outputSize,
	string targetHash)
{

	size_t inStrlength = inputSize;//strlen(input);
	//cout << "input string : [" << input << "]" << endl;
	int outputlength = HASH_LENGTH * gpuThreads;
	//	size_t outputSize = outputSize; //outputlength * sizeof(char);


	pOcl->inputBuffer = clCreateBuffer(pOcl->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, (inStrlength + 1) * sizeof(char), (void *)input, NULL);
	pOcl->outputNonceBuffer = clCreateBuffer(pOcl->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, targetHash.size() * sizeof(char), (void *)targetHash.c_str(), NULL);
	pOcl->outputBuffer = clCreateBuffer(pOcl->context, CL_MEM_WRITE_ONLY, outputSize, NULL, NULL);

	/*Step 8: Create kernel object */
	pOcl->kernel = clCreateKernel(pOcl->program, kernel_name, NULL);

	cl_int outStringlen = -1;
	//char* target = (char*)targetHash;//
	/*Step 9: Sets Kernel arguments.*/
	cl_int status = clSetKernelArg(pOcl->kernel, 0, sizeof(cl_mem), (void *)&pOcl->inputBuffer);
	status = clSetKernelArg(pOcl->kernel, 1, sizeof(cl_int), (void *)&inStrlength);
	status = clSetKernelArg(pOcl->kernel, 2, sizeof(cl_mem), (void *)&pOcl->outputBuffer);
	status = clSetKernelArg(pOcl->kernel, 3, sizeof(cl_int), (void *)&outStringlen);
	status = clSetKernelArg(pOcl->kernel, 4, sizeof(cl_ulong), (void *)&startIndex);
	status = clSetKernelArg(pOcl->kernel, 5, sizeof(cl_mem), (void *)&pOcl->outputNonceBuffer);


	/*Step 10: Running the kernel.*/
	size_t global_work_size[1] = { gpuThreads };
	status = clEnqueueNDRangeKernel(pOcl->commandQueue, pOcl->kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

	/*Step 11: Read the cout put back to host memory.*/
	status = clEnqueueReadBuffer(pOcl->commandQueue, pOcl->outputBuffer, CL_TRUE, 0, outputSize, output, 0, NULL, NULL);


	/*Relase input output cl buffers*/
	status = clReleaseMemObject(pOcl->inputBuffer);		//Release mem object.
	status = clReleaseMemObject(pOcl->outputBuffer);

	pOcl->inputBuffer = NULL;
	pOcl->outputBuffer = NULL;

	return (cl_int)outStringlen;

}