#include <stdio.h>
#include <stdlib.h>

#ifndef YU_TRACE_H
#define YU_TRACE_H
	
	#define SSD_PAGE2SECTOR (4096/512)

	void traceFile2print(FILE *fin);
	void traceFile2max(FILE *fin);
	void traceFile2blkno(FILE *fin);
#endif
