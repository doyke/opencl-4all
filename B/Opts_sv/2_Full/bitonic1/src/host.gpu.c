/* ********************************************************************************************* */
/* * C Template for Kernel Execution                                                           * */
/* * Author: André Bannwart Perina                                                             * */
/* ********************************************************************************************* */
/* * Copyright (c) 2017 André B. Perina                                                        * */
/* *                                                                                           * */
/* * Permission is hereby granted, free of charge, to any person obtaining a copy of this      * */
/* * software and associated documentation files (the "Software"), to deal in the Software     * */
/* * without restriction, including without limitation the rights to use, copy, modify,        * */
/* * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to        * */
/* * permit persons to whom the Software is furnished to do so, subject to the following       * */
/* * conditions:                                                                               * */
/* *                                                                                           * */
/* * The above copyright notice and this permission notice shall be included in all copies     * */
/* * or substantial portions of the Software.                                                  * */
/* *                                                                                           * */
/* * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,       * */
/* * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR  * */
/* * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE * */
/* * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      * */
/* * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER    * */
/* * DEALINGS IN THE SOFTWARE.                                                                 * */
/* ********************************************************************************************* */

#include <CL/opencl.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "common.h"

/**
 * @brief Header where pre/postamble macro functions should be located.
 *        Function headers:
 *            PREAMBLE(d_DstKey, d_DstKeySz, d_DstKeyC, d_DstKeyCSz, d_DstVal, d_DstValSz, d_DstValC, d_DstValCSz, d_SrcKey, d_SrcKeySz, d_SrcVal, d_SrcValSz);
 *            POSTAMBLE(d_DstKey, d_DstKeySz, d_DstKeyC, d_DstKeyCSz, d_DstVal, d_DstValSz, d_DstValC, d_DstValCSz, d_SrcKey, d_SrcKeySz, d_SrcVal, d_SrcValSz);
 *            LOOPPREAMBLE(d_DstKey, d_DstKeySz, d_DstKeyC, d_DstKeyCSz, d_DstVal, d_DstValSz, d_DstValC, d_DstValCSz, d_SrcKey, d_SrcKeySz, d_SrcVal, d_SrcValSz, loopFlag);
 *            LOOPPOSTAMBLE(d_DstKey, d_DstKeySz, d_DstKeyC, d_DstKeyCSz, d_DstVal, d_DstValSz, d_DstValC, d_DstValCSz, d_SrcKey, d_SrcKeySz, d_SrcVal, d_SrcValSz, loopFlag);
 *            CLEANUP(d_DstKey, d_DstKeySz, d_DstKeyC, d_DstKeyCSz, d_DstVal, d_DstValSz, d_DstValC, d_DstValCSz, d_SrcKey, d_SrcKeySz, d_SrcVal, d_SrcValSz);
 *        where:
 *            d_DstKey: variable (unsigned int *);
 *            d_DstKeySz: number of members in variable (unsigned int);
 *            d_DstKeyC: variable (unsigned int *);
 *            d_DstKeyCSz: number of members in variable (unsigned int);
 *            d_DstVal: variable (unsigned int *);
 *            d_DstValSz: number of members in variable (unsigned int);
 *            d_DstValC: variable (unsigned int *);
 *            d_DstValCSz: number of members in variable (unsigned int);
 *            d_SrcKey: variable (unsigned int *);
 *            d_SrcKeySz: number of members in variable (unsigned int);
 *            d_SrcVal: variable (unsigned int *);
 *            d_SrcValSz: number of members in variable (unsigned int);
 *            loopFlag: loop condition variable (bool).
 */
#include "prepostambles.h"

/**
 * @brief Test if two operands are outside an epsilon range.
 *
 * @param a First operand.
 * @param b Second operand.
 * @param e Epsilon value.
 */
#define TEST_EPSILON(a, b, e) (((a > b) && (a - b > e)) || ((b >= a) && (b - a > e)))

/**
 * @brief Standard statements for function error handling and printing.
 *
 * @param funcName Function name that failed.
 */
#define FUNCTION_ERROR_STATEMENTS(funcName) {\
	rv = EXIT_FAILURE;\
	PRINT_FAIL();\
	fprintf(stderr, "Error: %s failed with return code %d.\n", funcName, fRet);\
}

/**
 * @brief Standard statements for POSIX error handling and printing.
 *
 * @param arg Arbitrary string to the printed at the end of error string.
 */
#define POSIX_ERROR_STATEMENTS(arg) {\
	rv = EXIT_FAILURE;\
	PRINT_FAIL();\
	fprintf(stderr, "Error: %s: %s\n", strerror(errno), arg);\
}

int main(void) {
	/* Return variable */
	int rv = EXIT_SUCCESS;

	/* OpenCL and aux variables */
	int i = 0, j = 0;
	cl_int platformsLen, devicesLen, fRet;
	cl_platform_id *platforms = NULL;
	cl_device_id *devices = NULL;
	cl_context context = NULL;
	cl_command_queue queueBitonicsortlocal1 = NULL;
	FILE *programFile = NULL;
	long programSz;
	char *programContent = NULL;
	cl_int programRet;
	cl_program program = NULL;
	cl_kernel kernelBitonicsortlocal1 = NULL;
	bool loopFlag = false;
	bool invalidDataFound = false;
	long totalTime;
	struct timeval tThen, tNow, tDelta, tExecTime;
	timerclear(&tExecTime);
	cl_uint workDimBitonicsortlocal1 = 1;
	size_t globalSizeBitonicsortlocal1[1] = {
		2048
	};
	size_t localSizeBitonicsortlocal1[1] = {
		32
	};

	/* Input/output variables */
	unsigned int *d_DstKey = malloc(4096 * sizeof(unsigned int));
	unsigned int *d_DstKeyC = malloc(4096 * sizeof(unsigned int));
	cl_mem d_DstKeyK = NULL;
	unsigned int *d_DstVal = malloc(4096 * sizeof(unsigned int));
	unsigned int *d_DstValC = malloc(4096 * sizeof(unsigned int));
	cl_mem d_DstValK = NULL;
	unsigned int *d_SrcKey = malloc(4096 * sizeof(unsigned int));
	cl_mem d_SrcKeyK = NULL;
	unsigned int *d_SrcVal = malloc(4096 * sizeof(unsigned int));
	cl_mem d_SrcValK = NULL;

	/* Calling preamble function */
	PRINT_STEP("Calling preamble function...");
	PREAMBLE(d_DstKey, 4096, d_DstKeyC, 4096, d_DstVal, 4096, d_DstValC, 4096, d_SrcKey, 4096, d_SrcVal, 4096);
	PRINT_SUCCESS();

	/* Get platforms IDs */
	PRINT_STEP("Getting platforms IDs...");
	fRet = clGetPlatformIDs(0, NULL, &platformsLen);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clGetPlatformIDs"));
	platforms = malloc(platformsLen * sizeof(cl_platform_id));
	fRet = clGetPlatformIDs(platformsLen, platforms, NULL);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clGetPlatformIDs"));
	PRINT_SUCCESS();

	/* Get devices IDs for first platform availble */
	PRINT_STEP("Getting devices IDs for first platform...");
	fRet = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &devicesLen);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clGetDevicesIDs"));
	devices = malloc(devicesLen * sizeof(cl_device_id));
	fRet = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, devicesLen, devices, NULL);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clGetDevicesIDs"));
	PRINT_SUCCESS();

	/* Create context for first available device */
	PRINT_STEP("Creating context...");
	context = clCreateContext(NULL, 1, devices, NULL, NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateContext"));
	PRINT_SUCCESS();

	/* Create command queue for bitonicSortLocal1 kernel */
	PRINT_STEP("Creating command queue for \"bitonicSortLocal1\"...");
	queueBitonicsortlocal1 = clCreateCommandQueue(context, devices[0], 0, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateCommandQueue"));
	PRINT_SUCCESS();

	/* Open binary file */
	PRINT_STEP("Opening program binary...");
	programFile = fopen("kern.cl", "rb");
	ASSERT_CALL(programFile, POSIX_ERROR_STATEMENTS("kern.cl"));
	PRINT_SUCCESS();

	/* Get size and read file */
	PRINT_STEP("Reading program binary...");
	fseek(programFile, 0, SEEK_END);
	programSz = ftell(programFile);
	fseek(programFile, 0, SEEK_SET);
	programContent = malloc(programSz);
	fread(programContent, programSz, 1, programFile);
	fclose(programFile);
	programFile = NULL;
	PRINT_SUCCESS();

	/* Create program from source file */
	PRINT_STEP("Creating program from source...");
	program = clCreateProgramWithSource(context, 1, (const char **) &programContent, &programSz, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateProgramWithSource"));
	PRINT_SUCCESS();

	/* Build program */
	PRINT_STEP("Building program...");
	fRet = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clBuildProgram"));
	PRINT_SUCCESS();

	/* Create bitonicSortLocal1 kernel */
	PRINT_STEP("Creating kernel \"bitonicSortLocal1\" from program...");
	kernelBitonicsortlocal1 = clCreateKernel(program, "bitonicSortLocal1", &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateKernel"));
	PRINT_SUCCESS();

	/* Create input and output buffers */
	PRINT_STEP("Creating buffers...");
	d_DstKeyK = clCreateBuffer(context, CL_MEM_READ_WRITE, 4096 * sizeof(unsigned int), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (d_DstKeyK)"));
	d_DstValK = clCreateBuffer(context, CL_MEM_READ_WRITE, 4096 * sizeof(unsigned int), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (d_DstValK)"));
	d_SrcKeyK = clCreateBuffer(context, CL_MEM_READ_ONLY, 4096 * sizeof(unsigned int), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (d_SrcKeyK)"));
	d_SrcValK = clCreateBuffer(context, CL_MEM_READ_ONLY, 4096 * sizeof(unsigned int), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (d_SrcValK)"));
	PRINT_SUCCESS();

	/* Set kernel arguments for bitonicSortLocal1 */
	PRINT_STEP("Setting kernel arguments for \"bitonicSortLocal1\"...");
	fRet = clSetKernelArg(kernelBitonicsortlocal1, 0, sizeof(cl_mem), &d_DstKeyK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (d_DstKeyK)"));
	fRet = clSetKernelArg(kernelBitonicsortlocal1, 1, sizeof(cl_mem), &d_DstValK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (d_DstValK)"));
	fRet = clSetKernelArg(kernelBitonicsortlocal1, 2, sizeof(cl_mem), &d_SrcKeyK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (d_SrcKeyK)"));
	fRet = clSetKernelArg(kernelBitonicsortlocal1, 3, sizeof(cl_mem), &d_SrcValK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (d_SrcValK)"));
	PRINT_SUCCESS();

	do {
		/* Setting input and output buffers */
		PRINT_STEP("[%d] Setting buffers...", i);
		fRet = clEnqueueWriteBuffer(queueBitonicsortlocal1, d_DstKeyK, CL_TRUE, 0, 4096 * sizeof(unsigned int), d_DstKey, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (d_DstKeyK)"));
		fRet = clEnqueueWriteBuffer(queueBitonicsortlocal1, d_DstValK, CL_TRUE, 0, 4096 * sizeof(unsigned int), d_DstVal, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (d_DstValK)"));
		fRet = clEnqueueWriteBuffer(queueBitonicsortlocal1, d_SrcKeyK, CL_TRUE, 0, 4096 * sizeof(unsigned int), d_SrcKey, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (d_SrcKeyK)"));
		fRet = clEnqueueWriteBuffer(queueBitonicsortlocal1, d_SrcValK, CL_TRUE, 0, 4096 * sizeof(unsigned int), d_SrcVal, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (d_SrcValK)"));
		PRINT_SUCCESS();

		PRINT_STEP("[%d] Running kernels...", i);
		gettimeofday(&tThen, NULL);
		fRet = clEnqueueNDRangeKernel(queueBitonicsortlocal1, kernelBitonicsortlocal1, workDimBitonicsortlocal1, NULL, globalSizeBitonicsortlocal1, localSizeBitonicsortlocal1, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueNDRangeKernel"));
		clFinish(queueBitonicsortlocal1);
		gettimeofday(&tNow, NULL);
		PRINT_SUCCESS();

		/* Get output buffers */
		PRINT_STEP("[%d] Getting kernels arguments...", i);
		fRet = clEnqueueReadBuffer(queueBitonicsortlocal1, d_DstKeyK, CL_TRUE, 0, 4096 * sizeof(unsigned int), d_DstKey, 0, NULL, NULL);
		fRet = clEnqueueReadBuffer(queueBitonicsortlocal1, d_DstValK, CL_TRUE, 0, 4096 * sizeof(unsigned int), d_DstVal, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueReadBuffer"));
		PRINT_SUCCESS();

		timersub(&tNow, &tThen, &tDelta);
		timeradd(&tExecTime, &tDelta, &tExecTime);
		i++;
	} while(loopFlag);


	/* Print profiling results */
	totalTime = (1000000 * tExecTime.tv_sec) + tExecTime.tv_usec;
	printf("Elapsed time spent on kernels: %ld us; Average time per iteration: %lf us.\n", totalTime, totalTime / (double) i);

	/* Validate received data */
	PRINT_STEP("Validating received data...");
	for(i = 0; i < 4096; i++) {
		if(d_DstKeyC[i] != d_DstKey[i]) {
			if(!invalidDataFound) {
				PRINT_FAIL();
				invalidDataFound = true;
			}
			printf("Variable d_DstKey[%d]: expected %u got %u.\n", i, d_DstKeyC[i], d_DstKey[i]);
		}
	}
	for(i = 0; i < 4096; i++) {
		if(d_DstValC[i] != d_DstVal[i]) {
			if(!invalidDataFound) {
				PRINT_FAIL();
				invalidDataFound = true;
			}
			printf("Variable d_DstVal[%d]: expected %u got %u.\n", i, d_DstValC[i], d_DstVal[i]);
		}
	}
	if(!invalidDataFound)
		PRINT_SUCCESS();

_err:

	/* Dealloc buffers */
	if(d_DstKeyK)
		clReleaseMemObject(d_DstKeyK);
	if(d_DstValK)
		clReleaseMemObject(d_DstValK);
	if(d_SrcKeyK)
		clReleaseMemObject(d_SrcKeyK);
	if(d_SrcValK)
		clReleaseMemObject(d_SrcValK);

	/* Dealloc variables */
	free(d_DstKey);
	free(d_DstKeyC);
	free(d_DstVal);
	free(d_DstValC);
	free(d_SrcKey);
	free(d_SrcVal);

	/* Dealloc kernels */
	if(kernelBitonicsortlocal1)
		clReleaseKernel(kernelBitonicsortlocal1);

	/* Dealloc program */
	if(program)
		clReleaseProgram(program);
	if(programContent)
		free(programContent);
	if(programFile)
		fclose(programFile);

	/* Dealloc queues */
	if(queueBitonicsortlocal1)
		clReleaseCommandQueue(queueBitonicsortlocal1);

	/* Last OpenCL variables */
	if(context)
		clReleaseContext(context);
	if(devices)
		free(devices);
	if(platforms)
		free(platforms);


	return rv;
}
