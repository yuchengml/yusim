#ifndef YU_CACHE_H
#define YU_CACHE_H
#include <stdlib.h>
#include <stdio.h>
#include "yu_parameter.h"

	//SSD cache
	typedef struct SSD_cache {
		struct SSD_cache *pre;
		unsigned long ssd_blkno;
		unsigned long hdd_blkno;
		int flag;
		struct SSD_cache *next;
	} SSD_CACHE;

	static int cachingSpace[SSD_CACHING_SPACE_BY_BLOCKS];
	static SSD_CACHE *cachingTable = NULL;

	int insertCACHE(unsigned long *ssd_blk, unsigned long *hdd_blk, int reqFlag);
	SSD_CACHE *evictCACHEByLRU();
	SSD_CACHE *evictCACHE(unsigned long hdd_blk);
	SSD_CACHE *searchCACHE(unsigned long hdd_blk);
	void printCACHEByLRU();
	unsigned long ssdBlk2simSector(unsigned long ssd_blk);

#endif