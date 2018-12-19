#define PREAMBLE(r, rSz, s, sSz, sC, sCSz, loopCount) {\
	int _i, _j;\
	unsigned int _vars = 2;\
	char *_fileNames[] = {\
		"inputR",\
		"outputS"\
	};\
	void *_varsPointers[] = {\
		r,\
		sC\
	};\
	unsigned int _varsSizes[] = {\
		rSz,\
		sCSz\
	};\
	unsigned int _varsTypeSizes[] = {\
		1,\
		sizeof(short)\
	};\
\
	for(_i = 0; _i < _vars; _i++) {\
		FILE *ipf = fopen(_fileNames[_i], "rb");\
		fread(_varsPointers[_i], _varsTypeSizes[_i], _varsSizes[_i], ipf);\
		fclose(ipf);\
	}\
}