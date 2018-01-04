#include <stdio.h>
#include <stdlib.h>

#ifndef YU_TRACE_H
#define YU_TRACE_H
	
	#define SSD_PAGE2SECTOR 8
	#define NUM_OF_USER 1 //Hint:trace的userno是由1至n

	void traceFile2print(FILE *fin);
	void traceFile2max(FILE *fin);
	void traceFile2blkno(FILE *fin);


	typedef struct req {
		double arrivalTime;		//抵達時間
		unsigned devno;			//裝置編號(預設為0)
		unsigned long diskBlkno;//Block編號(根據Disksim格式)
		unsigned reqSize;		//Block連續數量(至少為1)
		unsigned reqFlag;		//讀:1;寫:0
		unsigned userno;		//使用者編號(1~n)
		double responseTime;	//反應時間(初始為0)
	} REQ;

	typedef struct tab {
		unsigned long diskBlkno;//Block編號(根據Disksim格式)
		struct tab *next;
	} TAB;

	void traceFile2workingSet(FILE *fin);

	void printProgress(unsigned long current, unsigned long total);
#endif
