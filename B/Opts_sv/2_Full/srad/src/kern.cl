#define fp float
#define SRAD_SIMD 2
#define NUMBER_THREADS 256

__attribute__((num_simd_work_items(SRAD_SIMD)))
__attribute__((reqd_work_group_size(NUMBER_THREADS,1,1)))
__kernel void srad_kernel(int           d_Nr,
                          long          d_Ne, 
                 __global int* restrict d_iN, 
                 __global int* restrict d_iS, 
                 __global int* restrict d_jE, 
                 __global int* restrict d_jW, 
                 __global fp*  restrict d_dN, 
                 __global fp*  restrict d_dS, 
                 __global fp*  restrict d_dE, 
                 __global fp*  restrict d_dW, 
                          fp            d_q0sqr, 
                 __global fp*  restrict d_c, 
                 __global fp*  restrict d_I)
{
	// indexes
	int bx = get_group_id(0);												// get current horizontal block index
	int tx = get_local_id(0);												// get current horizontal thread index
	int ei = bx * NUMBER_THREADS + tx;											// more threads than actual elements !!!
	int row;																// column, x position
	int col;																// row, y position

	// variables
	fp d_Jc;
	fp d_dN_loc, d_dS_loc, d_dW_loc, d_dE_loc;
	fp d_c_loc;
	fp d_G2, d_L, d_num, d_den, d_qsqr;
	
	// figure out row/col location in new matrix
	row = (ei + 1) % d_Nr - 1;												// (0-n) row
	col = (ei + 1) / d_Nr + 1 - 1;											// (0-n) column
	if((ei + 1) % d_Nr == 0)
	{
		row = d_Nr - 1;
		col = col - 1;
	}
	
	if(ei < d_Ne)
	{																	// make sure that only threads matching jobs run
		// directional derivatives, ICOV, diffusion coefficient
		d_Jc = d_I[ei];													// get value of the current element
		
		// directional derivatives (every element of IMAGE)(try to copy to shared memory or temp files)
		d_dN_loc = d_I[d_iN[row] + d_Nr * col] - d_Jc;							// north direction derivative
		d_dS_loc = d_I[d_iS[row] + d_Nr * col] - d_Jc;							// south direction derivative
		d_dW_loc = d_I[row + d_Nr * d_jW[col]] - d_Jc;							// west direction derivative
		d_dE_loc = d_I[row + d_Nr * d_jE[col]] - d_Jc;							// east direction derivative
	         
		// normalized discrete gradient mag squared (equ 52,53)
		d_G2 = (d_dN_loc * d_dN_loc + d_dS_loc * d_dS_loc + d_dW_loc * d_dW_loc + d_dE_loc * d_dE_loc) / (d_Jc * d_Jc);	// gradient (based on derivatives)
		
		// normalized discrete laplacian (equ 54)
		d_L = (d_dN_loc + d_dS_loc + d_dW_loc + d_dE_loc) / d_Jc;					// laplacian (based on derivatives)

		// ICOV (equ 31/35)
		d_num  = (0.5 * d_G2) - ((1.0 / 16.0) * (d_L * d_L)) ;						// num (based on gradient and laplacian)
		d_den  = 1 + (0.25 * d_L);											// den (based on laplacian)
		d_qsqr = d_num / (d_den * d_den);										// qsqr (based on num and den)
	 
		// diffusion coefficient (equ 33) (every element of IMAGE)
		d_den = (d_qsqr - d_q0sqr) / (d_q0sqr * (1 + d_q0sqr)) ;					// den (based on qsqr and q0sqr)
		d_c_loc = 1.0 / (1.0 + d_den) ;										// diffusion coefficient (based on den)
	    
		// saturate diffusion coefficient to 0-1 range
		if (d_c_loc < 0)													// if diffusion coefficient < 0
		{
			d_c_loc = 0;													// ... set to 0
		}
		else if (d_c_loc > 1)												// if diffusion coefficient > 1
		{
			d_c_loc = 1;													// ... set to 1
		}

		// save data to global memory
		d_dN[ei] = d_dN_loc; 
		d_dS[ei] = d_dS_loc; 
		d_dW[ei] = d_dW_loc; 
		d_dE[ei] = d_dE_loc;
		d_c[ei]  = d_c_loc;
	}
	
}