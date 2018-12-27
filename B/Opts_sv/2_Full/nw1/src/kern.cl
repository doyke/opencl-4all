/**
 * Copyright (c) 2018 Andre Bannwart Perina and others
 *
 * Adapted from
 * https://github.com/fpga-opencl-benchmarks/rodinia_fpga
 * Different licensing may apply, please check the repository documentation.
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

#define SCORE(i, j) input_itemsets_l[j + i * (BSIZE+1)]
#define REF(i, j)   reference_l[j + i * BSIZE]

#ifndef SSIZE
	#define SSIZE 4
#endif

#ifndef UNROLL
	#define UNROLL 1
#endif

#define BSIZE 16

int maximum(int a, int b, int c)
{
	int k;
	if(a <= b)
		k = b;
	else 
		k = a;

	if(k <= c)
		return(c);
	else
		return(k);
}

__attribute__((num_simd_work_items(SSIZE)))
__attribute__((reqd_work_group_size(BSIZE,1,1)))
__kernel void nw_kernel1(__global int* restrict reference_d, 
                         __global int* restrict input_itemsets_d, 
                                  int           cols,
                                  int           penalty,
                                  int           blk,
                                  int           offset_r,
                                  int           offset_c)
{  
	// Block index
	int bx = get_group_id(0);	

	// Thread index
	int tx = get_local_id(0);

	// Base elements
	int base = offset_r * cols + offset_c;
	
	// Local buffers
	__local int input_itemsets_l[(BSIZE + 1) * (BSIZE + 1)];
	__local int reference_l[BSIZE * BSIZE];

	int b_index_x = bx;
	int b_index_y = blk - 1 - bx;

	int index    = base + cols * BSIZE * b_index_y + BSIZE * b_index_x + tx + cols + 1;
	int index_n  = base + cols * BSIZE * b_index_y + BSIZE * b_index_x + tx + 1;
	int index_w  = base + cols * BSIZE * b_index_y + BSIZE * b_index_x + cols;
	int index_nw = base + cols * BSIZE * b_index_y + BSIZE * b_index_x;

	if (tx == 0)
	{
		SCORE(tx, 0) = input_itemsets_d[index_nw + tx];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	for (int ty = 0 ; ty < BSIZE ; ty++)
	{
		REF(ty, tx) = reference_d[index + cols * ty];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	SCORE((tx + 1), 0) = input_itemsets_d[index_w + cols * tx];

	barrier(CLK_LOCAL_MEM_FENCE);

	SCORE(0, (tx + 1)) = input_itemsets_d[index_n];

	barrier(CLK_LOCAL_MEM_FENCE);

	#pragma unroll UNROLL
	for(int m = 0 ; m < BSIZE ; m++)
	{
		if (tx <= m)
		{
			int t_index_x = tx + 1;
			int t_index_y = m - tx + 1;

			SCORE(t_index_y, t_index_x) = maximum( SCORE((t_index_y-1), (t_index_x-1)) + REF((t_index_y-1), (t_index_x-1))     ,
										    SCORE((t_index_y)  , (t_index_x-1)) - (penalty)        , SCORE((t_index_y-1),
										    (t_index_x)) - (penalty));
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	#pragma unroll UNROLL
	for(int m = BSIZE - 2 ; m >=0 ; m--)
	{
		if (tx <= m)
		{
			int t_index_x = tx + BSIZE - m ;
			int t_index_y = BSIZE - tx;

			SCORE(t_index_y, t_index_x) = maximum( SCORE((t_index_y-1), (t_index_x-1)) + REF((t_index_y-1), (t_index_x-1))     ,
		                                            SCORE((t_index_y)  , (t_index_x-1)) - (penalty)        , SCORE((t_index_y-1),
										    (t_index_x)) - (penalty));
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	for (int ty = 0 ; ty < BSIZE ; ty++)
	{
		input_itemsets_d[index + cols * ty] = SCORE((ty+1), (tx+1));
	}

	return;
}
