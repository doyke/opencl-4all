/**
 * Copyright (c) 2018 Andre Bannwart Perina and others
 *
 * Adapted from
 * rodinia_3.1/opencl/nw/nw.cl
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

#define BLOCK_SIZE 16

#define SCORE(i, j) input_itemsets_l[j + i * (BLOCK_SIZE+1)]
#define REF(i, j)   reference_l[j + i * BLOCK_SIZE]

int maximum( int a,
		 int b,
		 int c){

	int k;
	if( a <= b )
		k = b;
	else 
	k = a;

	if( k <=c )
	return(c);
	else
	return(k);
}

__attribute__((reqd_work_group_size(16,1,1)))
__kernel void 
nw_kernel1(__global int  * reference_d, 
		   __global int  * input_itemsets_d, 
		   __local	int  * input_itemsets_l,
		   __local	int  * reference_l,
           int cols,
           int penalty,
           int blk,
           int block_width,
           int worksize,
           int offset_r,
           int offset_c
    )
{  

	// Block index
    int bx = get_group_id(0);	
	//int bx = get_global_id(0)/BLOCK_SIZE;
   
    // Thread index
    int tx = get_local_id(0);
    
    // Base elements
    int base = offset_r * cols + offset_c;
    
    int b_index_x = bx;
	int b_index_y = blk - 1 - bx;
	
	
	int index   =   base + cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + tx + ( cols + 1 );
	int index_n   = base + cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + tx + ( 1 );
	int index_w   = base + cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x + ( cols );
	int index_nw =  base + cols * BLOCK_SIZE * b_index_y + BLOCK_SIZE * b_index_x;
   
    
	if (tx == 0){
		SCORE(tx, 0) = input_itemsets_d[index_nw + tx];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	for ( int ty = 0 ; ty < BLOCK_SIZE ; ty++)
		REF(ty, tx) =  reference_d[index + cols * ty];

	barrier(CLK_LOCAL_MEM_FENCE);

	SCORE((tx + 1), 0) = input_itemsets_d[index_w + cols * tx];

	barrier(CLK_LOCAL_MEM_FENCE);

	SCORE(0, (tx + 1)) = input_itemsets_d[index_n];
  
	barrier(CLK_LOCAL_MEM_FENCE);
	
	
	for( int m = 0 ; m < BLOCK_SIZE ; m++){
	
	  if ( tx <= m ){
	  
		  int t_index_x =  tx + 1;
		  int t_index_y =  m - tx + 1;
			
		  SCORE(t_index_y, t_index_x) = maximum( SCORE((t_index_y-1), (t_index_x-1)) + REF((t_index_y-1), (t_index_x-1)),
		                                         SCORE((t_index_y),   (t_index_x-1)) - (penalty), 
												 SCORE((t_index_y-1), (t_index_x))   - (penalty));
	  }
	  barrier(CLK_LOCAL_MEM_FENCE);
    }
    
     barrier(CLK_LOCAL_MEM_FENCE);
    
	for( int m = BLOCK_SIZE - 2 ; m >=0 ; m--){
   
	  if ( tx <= m){
 
		  int t_index_x =  tx + BLOCK_SIZE - m ;
		  int t_index_y =  BLOCK_SIZE - tx;

         SCORE(t_index_y, t_index_x) = maximum(  SCORE((t_index_y-1), (t_index_x-1)) + REF((t_index_y-1), (t_index_x-1)),
		                                         SCORE((t_index_y),   (t_index_x-1)) - (penalty), 
		 										 SCORE((t_index_y-1), (t_index_x))   - (penalty));
	   
	  }

	  barrier(CLK_LOCAL_MEM_FENCE);
	}
	

   for ( int ty = 0 ; ty < BLOCK_SIZE ; ty++)
     input_itemsets_d[index + cols * ty] = SCORE((ty+1), (tx+1));
    
    return;
   
}
