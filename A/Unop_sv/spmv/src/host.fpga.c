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
 *            PREAMBLE(val, valSz, vec, vecSz, cols, colsSz, rowDelimiters, rowDelimitersSz, dim, vecWidth, out, outSz, outC, outCSz);
 *            POSTAMBLE(val, valSz, vec, vecSz, cols, colsSz, rowDelimiters, rowDelimitersSz, dim, vecWidth, out, outSz, outC, outCSz);
 *            LOOPPREAMBLE(val, valSz, vec, vecSz, cols, colsSz, rowDelimiters, rowDelimitersSz, dim, vecWidth, out, outSz, outC, outCSz, loopFlag);
 *            LOOPPOSTAMBLE(val, valSz, vec, vecSz, cols, colsSz, rowDelimiters, rowDelimitersSz, dim, vecWidth, out, outSz, outC, outCSz, loopFlag);
 *            CLEANUP(val, valSz, vec, vecSz, cols, colsSz, rowDelimiters, rowDelimitersSz, dim, vecWidth, out, outSz, outC, outCSz);
 *        where:
 *            val: variable (float *);
 *            valSz: number of members in variable (unsigned int);
 *            vec: variable (float *);
 *            vecSz: number of members in variable (unsigned int);
 *            cols: variable (int *);
 *            colsSz: number of members in variable (unsigned int);
 *            rowDelimiters: variable (int *);
 *            rowDelimitersSz: number of members in variable (unsigned int);
 *            dim: variable (int);
 *            vecWidth: variable (int);
 *            out: variable (float *);
 *            outSz: number of members in variable (unsigned int);
 *            outC: variable (float *);
 *            outCSz: number of members in variable (unsigned int);
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
	cl_command_queue queueSpmv_Csr_Vector_Kernel = NULL;
	FILE *programFile = NULL;
	long programSz;
	char *programContent = NULL;
	cl_int programRet;
	cl_program program = NULL;
	cl_kernel kernelSpmv_Csr_Vector_Kernel = NULL;
	bool loopFlag = false;
	bool invalidDataFound = false;
	struct timeval tThen, tNow, tDelta, tExecTime;
	timerclear(&tExecTime);
	cl_uint workDimSpmv_Csr_Vector_Kernel = 1;
	size_t globalSizeSpmv_Csr_Vector_Kernel[1] = {
		131072
	};
	size_t localSizeSpmv_Csr_Vector_Kernel[1] = {
		128
	};

	/* Input/output variables */
	float *val = malloc(10485 * sizeof(float));
	cl_mem valK = NULL;
	float *vec = malloc(1024 * sizeof(float));
	cl_mem vecK = NULL;
	int *cols = malloc(10485 * sizeof(int));
	cl_mem colsK = NULL;
	int *rowDelimiters = malloc(1025 * sizeof(int));
	cl_mem rowDelimitersK = NULL;
	int dim = 1024;
	int vecWidth = 128;
	float *out = malloc(1024 * sizeof(float));
	float *outC = malloc(1024 * sizeof(float));
	double outEpsilon = 0.02;
	cl_mem outK = NULL;

	/* Calling preamble function */
	PRINT_STEP("Calling preamble function...");
	PREAMBLE(val, 10485, vec, 1024, cols, 10485, rowDelimiters, 1025, dim, vecWidth, out, 1024, outC, 1024);
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

	/* Create command queue for spmv_csr_vector_kernel kernel */
	PRINT_STEP("Creating command queue for \"spmv_csr_vector_kernel\"...");
	queueSpmv_Csr_Vector_Kernel = clCreateCommandQueue(context, devices[0], 0, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateCommandQueue"));
	PRINT_SUCCESS();

	/* Open binary file */
	PRINT_STEP("Opening program binary...");
	programFile = fopen("program.aocx", "rb");
	ASSERT_CALL(programFile, POSIX_ERROR_STATEMENTS("program.aocx"));
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

	/* Create program from binary file */
	PRINT_STEP("Creating program from binary...");
	program = clCreateProgramWithBinary(context, 1, devices, &programSz, (const unsigned char **) &programContent, &programRet, &fRet);
	ASSERT_CALL(CL_SUCCESS == programRet, FUNCTION_ERROR_STATEMENTS("clCreateProgramWithBinary (when loading binary)"));
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateProgramWithBinary"));
	PRINT_SUCCESS();

	/* Build program */
	PRINT_STEP("Building program...");
	fRet = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clBuildProgram"));
	PRINT_SUCCESS();

	/* Create spmv_csr_vector_kernel kernel */
	PRINT_STEP("Creating kernel \"spmv_csr_vector_kernel\" from program...");
	kernelSpmv_Csr_Vector_Kernel = clCreateKernel(program, "spmv_csr_vector_kernel", &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateKernel"));
	PRINT_SUCCESS();

	/* Create input and output buffers */
	PRINT_STEP("Creating buffers...");
	valK = clCreateBuffer(context, CL_MEM_READ_ONLY, 10485 * sizeof(float), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (valK)"));
	vecK = clCreateBuffer(context, CL_MEM_READ_ONLY, 1024 * sizeof(float), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (vecK)"));
	colsK = clCreateBuffer(context, CL_MEM_READ_ONLY, 10485 * sizeof(int), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (colsK)"));
	rowDelimitersK = clCreateBuffer(context, CL_MEM_READ_ONLY, 1025 * sizeof(int), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (rowDelimitersK)"));
	outK = clCreateBuffer(context, CL_MEM_READ_WRITE, 1024 * sizeof(float), NULL, &fRet);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clCreateBuffer (outK)"));
	PRINT_SUCCESS();

	/* Set kernel arguments for spmv_csr_vector_kernel */
	PRINT_STEP("Setting kernel arguments for \"spmv_csr_vector_kernel\"...");
	fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 0, sizeof(cl_mem), &valK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (valK)"));
	fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 1, sizeof(cl_mem), &vecK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (vecK)"));
	fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 2, sizeof(cl_mem), &colsK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (colsK)"));
	fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 3, sizeof(cl_mem), &rowDelimitersK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (rowDelimitersK)"));
	fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 4, sizeof(int), &dim);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (dim)"));
	fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 5, sizeof(int), &vecWidth);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (vecWidth)"));
	fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 6, sizeof(cl_mem), &outK);
	ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (outK)"));
	PRINT_SUCCESS();

	do {
		/* Setting input and output buffers */
		PRINT_STEP("[%d] Setting buffers...", i);
		fRet = clEnqueueWriteBuffer(queueSpmv_Csr_Vector_Kernel, valK, CL_TRUE, 0, 10485 * sizeof(float), val, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (valK)"));
		fRet = clEnqueueWriteBuffer(queueSpmv_Csr_Vector_Kernel, vecK, CL_TRUE, 0, 1024 * sizeof(float), vec, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (vecK)"));
		fRet = clEnqueueWriteBuffer(queueSpmv_Csr_Vector_Kernel, colsK, CL_TRUE, 0, 10485 * sizeof(int), cols, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (colsK)"));
		fRet = clEnqueueWriteBuffer(queueSpmv_Csr_Vector_Kernel, rowDelimitersK, CL_TRUE, 0, 1025 * sizeof(int), rowDelimiters, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (rowDelimitersK)"));
		fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 4, sizeof(int), &dim);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (dim)"));
		fRet = clSetKernelArg(kernelSpmv_Csr_Vector_Kernel, 5, sizeof(int), &vecWidth);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clSetKernelArg (vecWidth)"));
		fRet = clEnqueueWriteBuffer(queueSpmv_Csr_Vector_Kernel, outK, CL_TRUE, 0, 1024 * sizeof(float), out, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueWriteBuffer (outK)"));
		PRINT_SUCCESS();

		PRINT_STEP("[%d] Running kernels...", i);
		gettimeofday(&tThen, NULL);
		fRet = clEnqueueNDRangeKernel(queueSpmv_Csr_Vector_Kernel, kernelSpmv_Csr_Vector_Kernel, workDimSpmv_Csr_Vector_Kernel, NULL, globalSizeSpmv_Csr_Vector_Kernel, localSizeSpmv_Csr_Vector_Kernel, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueNDRangeKernel"));
		clFinish(queueSpmv_Csr_Vector_Kernel);
		gettimeofday(&tNow, NULL);
		PRINT_SUCCESS();

		/* Get output buffers */
		PRINT_STEP("[%d] Getting kernels arguments...", i);
		fRet = clEnqueueReadBuffer(queueSpmv_Csr_Vector_Kernel, outK, CL_TRUE, 0, 1024 * sizeof(float), out, 0, NULL, NULL);
		ASSERT_CALL(CL_SUCCESS == fRet, FUNCTION_ERROR_STATEMENTS("clEnqueueReadBuffer"));
		PRINT_SUCCESS();

		timersub(&tNow, &tThen, &tDelta);
		timeradd(&tExecTime, &tDelta, &tExecTime);
		i++;
	} while(loopFlag);


	/* Print profiling results */
	long totalTime = (1000000 * tExecTime.tv_sec) + tExecTime.tv_usec;
	printf("Elapsed time spent on kernels: %ld us; Average time per iteration: %lf us.\n", totalTime, totalTime / (double) i);

	/* Validate received data */
	PRINT_STEP("Validating received data...");
	for(i = 0; i < 1024; i++) {
		if(TEST_EPSILON(outC[i],  out[i], outEpsilon)) {
			if(!invalidDataFound) {
				PRINT_FAIL();
				invalidDataFound = true;
			}
			printf("Variable out[%d]: expected %f got %f (with epsilon).\n", i, outC[i], out[i]);
		}
	}
	if(!invalidDataFound)
		PRINT_SUCCESS();

_err:

	/* Dealloc buffers */
	if(valK)
		clReleaseMemObject(valK);
	if(vecK)
		clReleaseMemObject(vecK);
	if(colsK)
		clReleaseMemObject(colsK);
	if(rowDelimitersK)
		clReleaseMemObject(rowDelimitersK);
	if(outK)
		clReleaseMemObject(outK);

	/* Dealloc variables */
	free(val);
	free(vec);
	free(cols);
	free(rowDelimiters);
	free(out);
	free(outC);

	/* Dealloc kernels */
	if(kernelSpmv_Csr_Vector_Kernel)
		clReleaseKernel(kernelSpmv_Csr_Vector_Kernel);

	/* Dealloc program */
	if(program)
		clReleaseProgram(program);
	if(programContent)
		free(programContent);
	if(programFile)
		fclose(programFile);

	/* Dealloc queues */
	if(queueSpmv_Csr_Vector_Kernel)
		clReleaseCommandQueue(queueSpmv_Csr_Vector_Kernel);

	/* Last OpenCL variables */
	if(context)
		clReleaseContext(context);
	if(devices)
		free(devices);
	if(platforms)
		free(platforms);


	return rv;
}