// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "Windows.h"
#define SUCCESS 0
#define FAILURE 1

const unsigned long int GPUTHREADS = 1000;
const unsigned int HASH_LENGTH = 64;
using namespace std;

typedef struct {
	cl_kernel kernel;
	cl_program program;
	cl_mem inputBuffer;
	cl_mem outputBuffer;
	cl_command_queue commandQueue;
	cl_context context;
	cl_device_id        *devices;
} OCL_INSTANCE, *POCL_INSTANCE;



//////// 
///initialize OCL_INSTANCE
OCL_INSTANCE g_oclInstance;



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


int RunGpuCode_OpenCL(const char *filename,
	const char* input,
	char* output,
	const char* kernel_name
)
{
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
	cl_context context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

	/*Step 4: Creating command queue associate with the context.*/
	cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

	/*Step 5: Create program object */

	string sourceStr;
	status = convertToString(filename, sourceStr);
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = { strlen(source) };
	cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);

	/*Step 6: Build program. */
	status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
	if (status < 0)
	{
		cout << "Kernel has syntax errors, compilation failed. Err = " << status << endl;
		return 0;
	}

	size_t inStrlength = strlen(input);
	//cout << "input string : [" << input << "]" << endl;
	int outputlength = HASH_LENGTH * GPUTHREADS;
	size_t outputSize = outputlength * sizeof(char);

	cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, (inStrlength + 1) * sizeof(char), (void *)input, NULL);
	cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, outputSize, NULL, NULL);
	//cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, (outputSize + 1) * sizeof(char), NULL, NULL);

	/*Step 8: Create kernel object */
	cl_kernel kernel = clCreateKernel(program, kernel_name, NULL);

	/*Step 9: Sets Kernel arguments.*/
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
	status = clSetKernelArg(kernel, 1, sizeof(cl_int), (void *)&inStrlength);
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&outputBuffer);
	status = clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&outputSize);

	/*Step 10: Running the kernel.*/
	size_t global_work_size[1] = { GPUTHREADS };
	status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

	/*Step 11: Read the cout put back to host memory.*/
	status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, outputSize, output, 0, NULL, NULL);


	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(kernel);				//Release kernel.
	status = clReleaseProgram(program);				//Release the program object.
	status = clReleaseMemObject(inputBuffer);		//Release mem object.
	status = clReleaseMemObject(outputBuffer);
	status = clReleaseCommandQueue(commandQueue);	//Release  Command queue.
	status = clReleaseContext(context);				//Release context.


	if (devices != NULL)
	{
		free(devices);
		devices = NULL;
	}

	return status;
}

cl_int ReleaseOCL(OCL_INSTANCE* pOcl)
{
	/*Step 12: Clean the resources.*/
	cl_int status = clReleaseKernel(pOcl->kernel);		//Release kernel.
	status = clReleaseProgram(pOcl->program);			//Release the program object.
	status = clReleaseMemObject(pOcl->inputBuffer);		//Release mem object.
	status = clReleaseMemObject(pOcl->outputBuffer);
	status = clReleaseCommandQueue(pOcl->commandQueue);	//Release  Command queue.
	status = clReleaseContext(pOcl->context);			//Release context.

	pOcl->inputBuffer = NULL;
	pOcl->outputBuffer = NULL;
	
	if (pOcl->devices != NULL)
	{
		free(pOcl->devices);
		pOcl->devices = NULL;
	}

	return status;
}


BOOL WINAPI consoleHandler(DWORD signal) {

	if (signal == CTRL_C_EVENT)
		printf("Ctrl-C handled\n"); // do cleanup

	ReleaseOCL(&g_oclInstance);

	exit(0);
	return TRUE;
}


cl_int InitializeOCL(OCL_INSTANCE *pOcl, 
						const char *filename,
						const char* input,
						unsigned char* output )
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
		return 0;
	}


	size_t inStrlength = strlen(input);
	//cout << "input string : [" << input << "]" << endl;
	int outputlength = HASH_LENGTH * GPUTHREADS;
	size_t outputSize = outputlength * sizeof(char);

	//pOcl->inputBuffer = clCreateBuffer(pOcl->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, (inStrlength + 1) * sizeof(char), (void *)input, NULL);
	//pOcl->outputBuffer = clCreateBuffer(pOcl->context, CL_MEM_WRITE_ONLY, outputSize, NULL, NULL);


	return status;
}

cl_int RunGpu_Loads( POCL_INSTANCE pOcl,
									const char* input,
									char* output,
									const char* kernel_name )
{

	size_t inStrlength = strlen(input);
	//cout << "input string : [" << input << "]" << endl;
	int outputlength = HASH_LENGTH * GPUTHREADS;
	size_t outputSize = outputlength * sizeof(char);

	pOcl->inputBuffer = clCreateBuffer(pOcl->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, (inStrlength + 1) * sizeof(char), (void *)input, NULL);
	pOcl->outputBuffer = clCreateBuffer(pOcl->context, CL_MEM_WRITE_ONLY, outputSize, NULL, NULL);

	/*Step 8: Create kernel object */
	pOcl->kernel = clCreateKernel(pOcl->program, kernel_name, NULL);

	/*Step 9: Sets Kernel arguments.*/
	cl_int status = clSetKernelArg(pOcl->kernel, 0, sizeof(cl_mem), (void *)&pOcl->inputBuffer);
	status = clSetKernelArg(pOcl->kernel, 1, sizeof(cl_int), (void *)&inStrlength);
	status = clSetKernelArg(pOcl->kernel, 2, sizeof(cl_mem), (void *)&pOcl->outputBuffer);
	status = clSetKernelArg(pOcl->kernel, 3, sizeof(cl_int), (void *)&outputSize);

	/*Step 10: Running the kernel.*/
	size_t global_work_size[1] = { GPUTHREADS };
	status = clEnqueueNDRangeKernel(pOcl->commandQueue, pOcl->kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

	/*Step 11: Read the cout put back to host memory.*/
	status = clEnqueueReadBuffer(pOcl->commandQueue, pOcl->outputBuffer, CL_TRUE, 0, outputSize, output, 0, NULL, NULL);


	/*Relase input output cl buffers*/
	status = clReleaseMemObject(pOcl->inputBuffer);		//Release mem object.
	status = clReleaseMemObject(pOcl->outputBuffer);

	pOcl->inputBuffer = NULL;
	pOcl->outputBuffer = NULL;
	return status;

}





int main(int argc, char* argv[])
{

	const char *filename = "GpuSha256_Kernel.cl.cpp";

	int outputSize = 64 * GPUTHREADS;

	unsigned char* output = (unsigned char *)malloc(outputSize);

	const char* kernel_name = "sha256";

	std::vector <string> vStrings;

	string shello = "Hello";

	char bff[64] = "\0";

	UINT64 start = 0;// 0x3f522c - 1000;
	

	cout << "\nGPUTHREADS #  " << GPUTHREADS << endl;


	memset(&g_oclInstance, 0, sizeof(g_oclInstance));

	int ts = GetTickCount();

	

	std::vector <string> hashCollection;

	long int tsNonce = GetTickCount();
	string nonce;
	string  newstr=shello;

	
	//UINT64 counter = 0;
	//for (; counter < GPUTHREADS; ++counter)
	//{
	//	memset(bff, '\0', 64);
	//	sprintf(bff, "%x;", start + counter);
	//	string tmp = shello;
	//	tmp.append(string(bff));
	//	newstr = newstr.append(tmp);
	//	vStrings.push_back(tmp);
	//}

	cl_int	status = InitializeOCL(&g_oclInstance, filename, newstr.c_str(), output);

	//newstr.clear();

	//vStrings.clear();

	while( hashCollection.size() <= 0  )
	{

		newstr.clear();
		vStrings.clear();

		UINT64 counter = 0;
		for (; counter < GPUTHREADS; ++counter)
		{
			memset(bff, '\0', 64);
			sprintf(bff, "%x;", start+counter);
			string tmp = shello;
			tmp.append(string(bff));
			newstr = newstr.append(tmp);
			vStrings.push_back(tmp);
		}

		start += counter;

		int inlength = newstr.length();
		memset(output, '\0', outputSize);
		int ts = GetTickCount();
		status = RunGpu_Loads(&g_oclInstance, newstr.c_str(), (char*)output, kernel_name);

		int te = GetTickCount();

		//cout << "\nEnd Counter = " << te - ts << endl;

		char buff[65] = { '\0' };

		//ts = GetTickCount();
		for (int k = 0; k < GPUTHREADS; k++)
		{
			unsigned char* travStart = &output[k * 64];

			if (travStart[0] == 0x00 )
				if(travStart[2] == 0x00 )
					if(travStart[4] == 0x00 )
					{

						sprintf(buff, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
							travStart[0], travStart[2], travStart[4], travStart[6], travStart[8],
							travStart[10], travStart[12], travStart[14], travStart[16], travStart[18],
							travStart[20], travStart[22], travStart[24], travStart[26], travStart[28],
							travStart[30], travStart[32], travStart[34], travStart[36], travStart[38],
							travStart[40], travStart[42], travStart[44], travStart[46], travStart[48],
							travStart[50], travStart[52], travStart[54], travStart[56], travStart[58],
							travStart[60], travStart[62]
						);

						buff[64] = '\0';
						cout << "\n This is '4x0' ***SHA for  = " << string(buff);
						cout << "\n Number = " << vStrings[k] << endl;

						hashCollection.push_back(string(buff));
						nonce = vStrings[k];
						break;

					}
			

		}
		
		//cout << "\n 0x"<<hex << start<<dec <<" - decimal_start = " << start << endl;

	
	}

	long int teNonce = GetTickCount();

	cout << "\n Nonce found in  time = " << teNonce - tsNonce << endl;
	cout << "\n Nonce = 0x" <<hex<< start << endl;

	if (output != NULL)
	{
		free(output);
		output = NULL;
	}


	ReleaseOCL(&g_oclInstance);

	cout << endl;

	return SUCCESS;
}

