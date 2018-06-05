/*
Author : Anshul Yadav
email : yadav26@gmail.com
LinkedIn: https://www.linkedin.com/in/anshul-yadav-2289b734/
*/

#pragma once

#include "stdafx.h"

#define SUCCESS 0
#define FAILURE 1

const unsigned long int GPUTHREADS = 100000;
const unsigned int HASH_LENGTH = 64;

typedef struct {
	cl_kernel kernel;
	cl_program program;
	cl_mem inputBuffer;
	cl_mem inputTargetBuffer;
	cl_mem outputBuffer;
	cl_command_queue commandQueue;
	cl_context context;
	cl_device_id        *devices;

} OCL_INSTANCE, *POCL_INSTANCE;


int convertToString(const char *filename, std::string& s);

cl_int ReleaseOCL(OCL_INSTANCE* pOcl);

cl_int InitializeOCL(OCL_INSTANCE *pOcl,
	const char *filename,
	const char* input,
	unsigned char* output);

cl_int RunGpu_Loads(POCL_INSTANCE pOcl,
	const char* input,
	char* output,
	const char* kernel_name,
	cl_ulong startIndex,
	cl_uint outputSize,
	string targetHash);
