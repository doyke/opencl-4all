#define LOCAL_SIZE_LIMIT 64

inline void ComparatorLocal(
	__local uint *keyA,
	__local uint *valA,
	__local uint *keyB,
	__local uint *valB,
	uint arrowDir
){
	if((*keyA > *keyB) == arrowDir) {
		uint t;
		t = *keyA; *keyA = *keyB; *keyB = t;
		t = *valA; *valA = *valB; *valB = t;
	}
}

// Bitonic merge iteration for 'stride' >= LOCAL_SIZE_LIMIT
__kernel __attribute__((reqd_work_group_size(LOCAL_SIZE_LIMIT / 2, 1, 1)))
void bitonicMergeGlobal(
	__global uint *d_DstKey,
	__global uint *d_DstVal,
	__global uint *d_SrcKey,
	__global uint *d_SrcVal,
	uint arrayLength,
	uint size,
	uint stride,
	uint sortDir
){
	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint global_comparatorI[LOCAL_SIZE_LIMIT / 2];
	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint comparatorI[LOCAL_SIZE_LIMIT / 2];

	// Bitonic merge
	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint dir[LOCAL_SIZE_LIMIT / 2];
	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint pos[LOCAL_SIZE_LIMIT / 2];

	/////////////////////////////////////////////////////////
	global_comparatorI[get_local_id(0)] = get_global_id(0);

	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(0) == 0) {
#pragma unroll 4
		for(int m = 0; m < LOCAL_SIZE_LIMIT / 2; m++) {
			comparatorI[m] = global_comparatorI[m] & (arrayLength / 2 - 1);
			dir[m] = sortDir ^ ((comparatorI[m] & (size / 2)) != 0);
			pos[m] = 2 * global_comparatorI[m] - (global_comparatorI[m] & (stride - 1));
		}
	}
	////////////////////////////////////////////////////// 

	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint keyA[LOCAL_SIZE_LIMIT / 2];
	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint valA[LOCAL_SIZE_LIMIT / 2];
	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint keyB[LOCAL_SIZE_LIMIT / 2];
	__attribute__((numbanks(LOCAL_SIZE_LIMIT / 2), bankwidth(4)))
	__local uint valB[LOCAL_SIZE_LIMIT / 2];

	///////////////////////////////////////////////////////////////////////////////////////////
	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(0) == 0) {
#pragma unroll 4
		for(int i = 0; i < LOCAL_SIZE_LIMIT / 2; i++) {
			uint pos_i = pos[i];
			keyA[i] = d_SrcKey[pos_i +	  0];
			valA[i] = d_SrcVal[pos_i +	  0];
			keyB[i] = d_SrcKey[pos_i + stride];
			valB[i] = d_SrcVal[pos_i + stride];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	//////////////////////////////////////////////////////////////
	uint dir_ = dir[get_local_id(0)];

	ComparatorLocal(
		&keyA[get_local_id(0)], &valA[get_local_id(0)],
		&keyB[get_local_id(0)], &valB[get_local_id(0)],
		dir_
	);

	barrier(CLK_LOCAL_MEM_FENCE);
	/////////////////////////////////////////////////////

	if(get_local_id(0) == 0) {
#pragma unroll 4
		for(int j = 0; j < LOCAL_SIZE_LIMIT / 2; j++) {
			uint pos_o = pos[j];
			d_DstKey[pos_o +	  0] = keyA[j];
			d_DstVal[pos_o +	  0] = valA[j];
			d_DstKey[pos_o + stride] = keyB[j];
			d_DstVal[pos_o + stride] = valB[j];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);
}