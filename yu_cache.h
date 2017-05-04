#ifndef YU_CACHE_H
#define YU_CACHE_H
#include <stdlib.h>
#include <stdio.h>
#include "yu_parameter.h"
#include "yu_debug.h"

	/*STRUCTURE DEFINITION: SSD CACHE*/
	typedef struct SSD_cache {
		struct SSD_cache *pre;		//指向前一個CACHE BLOCK
		unsigned long ssd_blkno;	//SSD Block Number
		unsigned long hdd_blkno;	//HDD Block Number
		int dirtyFlag;				//標記是否為Dirty Block
		struct SSD_cache *next;
	} SSD_CACHE;

	/*SSD CACHE TABLE*/
	static SSD_CACHE *cachingTable = NULL;
	static unsigned long freeCount = SSD_CACHING_SPACE_BY_BLOCKS;
	/*CACHING SPACE*/
	//記錄實際的Cache狀態(Dirty, Clean or Free)
	int cachingSpace[SSD_CACHING_SPACE_BY_BLOCKS];

	/*INSERT CACHE TABLE*/
	int insertCACHE(unsigned long *ssd_blk, unsigned long *hdd_blk, int reqFlag);
	/*CACHE EVICTION:LRU*/
	SSD_CACHE *evictCACHEByLRU();
	/*CACHE EVICTION POLICY:SPECIFIC HDD Block*/
	SSD_CACHE *evictCACHE(unsigned long hdd_blk);
	/*SEARCH CACHE*/
	SSD_CACHE *searchCACHE(unsigned long hdd_blk);
	/*CHECK CACHE FULL*/
	int isFullCACHE();
	/*GET FREE CACHE*/
	unsigned long getFreeCACHE();
	/*CACHING TABLE OUTPUT*/
	void printCACHEByLRU();
	
#endif