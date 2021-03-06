/**
 * Copyright (c) 2018 Andre Bannwart Perina and others
 *
 * Adapted from
 * rodinia_3.1/opencl/bfs/Kernels.cl
 * Different licensing may apply, please check Rodinia documentation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics: enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics: enable


//Sungpack Hong, Sang Kyun Kim, Tayo Oguntebi, and Kunle Olukotun. 2011.
//Accelerating CUDA graph algorithms at maximum warp.
//In Proceedings of the 16th ACM symposium on Principles and practice of
//parallel programming (PPoPP '11). ACM, New York, NY, USA, 267-276.
// ****************************************************************************
// Function: BFS_kernel_warp
//
// Purpose:
//   Perform BFS on the given graph
//
// Arguments:
//   levels: array that stores the level of vertices
//   edgeArray: array that gives offset of a vertex in edgeArrayAux
//   edgeArrayAux: array that gives the edge list of a vertex
//   W_SZ: the warp size to use to process vertices
//   CHUNK_SZ: the number of vertices each warp processes
//   numVertices: number of vertices in the given graph
//   curr: the current BFS level
//   flag: set when more vertices remain to be traversed
//
// Returns:  nothing
//
// Programmer: Aditya Sarwade
// Creation: June 16, 2011
//
// Modifications:
//
// ****************************************************************************
__attribute__((reqd_work_group_size(8192,1,1)))
__kernel void BFS_kernel_warp(
        __global unsigned int *levels,
        __global unsigned int *edgeArray,
        __global unsigned int *edgeArrayAux,
        int W_SZ,
        int CHUNK_SZ,
        unsigned int numVertices,
        int curr,
        __global int *flag)
{

    int tid = get_global_id(0);
    int W_OFF = tid % W_SZ;
    int W_ID = tid / W_SZ;
    int v1= W_ID * CHUNK_SZ;
    int chk_sz=CHUNK_SZ+1;

    if((v1+CHUNK_SZ)>=numVertices)
    {
        chk_sz =  numVertices-v1+1;//(v1+CHUNK_SZ) - numVertices;
        if(chk_sz<0)
            chk_sz=0;
    }

    //each warp processes nodes one by one
    for(int v=v1; v< chk_sz-1+v1; v++)
    {
        if(levels[v] == curr)
        {
            unsigned int num_nbr = edgeArray[v+1]-edgeArray[v];
            unsigned int nbr_off = edgeArray[v];
            for(int i=W_OFF; i<num_nbr; i+=W_SZ)
            {
               int v = edgeArrayAux[i + nbr_off];
               if(levels[v]==UINT_MAX)
               {
                    levels[v] = curr + 1;
                    *flag = 1;
               }
            }
        }
    }
}
