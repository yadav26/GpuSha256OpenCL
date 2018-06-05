/*
Author : Anshul Yadav
email : yadav26@gmail.com
LinkedIn: https://www.linkedin.com/in/anshul-yadav-2289b734/
*/

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

using namespace std;


string shello = "hello";
const unsigned long int GPUTHREADS = 10;
const unsigned int HASH_LENGTH = 64;


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



int RunGpuCode_OpenCL( const char *filename,
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

int main(int argc, char* argv[])
{

	const char *filename = "GpuSha256_Kernel.cl.cpp";
	
	int outputSize = 64 * GPUTHREADS;

	unsigned char* output = (unsigned char *)malloc(outputSize);
	
	const char* kernel_name = "sha256";
	
	std::vector <string> vStrings;


	string  newstr;
	char bff[16] = "\0";
	int startoffset = 0; 
	for (int counter = startoffset; counter < GPUTHREADS+ startoffset; ++counter)
	{
		sprintf(bff, "%x;", counter);
		string tmp = shello;
		tmp.append(string(bff));
		newstr = newstr.append(tmp);
		vStrings.push_back(tmp);
	}

	int inlength = newstr.length();
	
	cout << "\nExpect to calculate hash'es #  " << GPUTHREADS << endl;

	int ts = GetTickCount();
	int retStatus = RunGpuCode_OpenCL( filename, newstr.c_str(), (char*)output, kernel_name);
	int te = GetTickCount();

	//cout << "\nEnd Counter = " << te - ts << endl;
	
	std::vector <string> hashCollection;
	
	std::vector <string> ::iterator itr = vStrings.begin();
	
	char buff[65] = { '\0' };

	ts = GetTickCount();
	
	for (int k = 0; k < GPUTHREADS; k++)
	{
		

		unsigned char* travStart = &output[k * 64];

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

		string tmp = buff;

		hashCollection.push_back(tmp);	

	}

	te = GetTickCount();

	//ofstream ofs("hash.out", std::ofstream::out); 

	//std::vector <string> ::iterator hitr = hashCollection.begin();
	//while ( itr != vStrings.end())
	//{
	//	ofs << *itr << "," << *hitr << endl;
	//	itr++;
	//	hitr++;
	//}

	//ofs.close();
	//cout << "\n Loop Assignment in Vector = " << te - ts << endl;

	cout << "\n Got String collection Vector length = " << hashCollection.size() << endl;

	if (output != NULL)
	{
		free(output);
		output = NULL;
	}

	cout << endl;
	
	return SUCCESS;
}

