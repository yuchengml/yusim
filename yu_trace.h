#include <stdio.h>
#include <stdlib.h>

#ifndef YU_TRACE_H
#define YU_TRACE_H
	
	#define SSD_PAGE2SECTOR (4096/512)
	#define NUM_OF_USER 1 //Hint:trace的userno是由1至n

	void traceFile2print(FILE *fin);
	void traceFile2max(FILE *fin);
	void traceFile2blkno(FILE *fin);
#endif
