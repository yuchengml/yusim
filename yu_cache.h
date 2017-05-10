#ifndef YU_CACHE_H
#define YU_CACHE_H
#include <stdlib.h>
#include <stdio.h>
#include "yu_parameter.h"
#include "yu_credit.h"
#include "yu_debug.h"

	/*STRUCTURE DEFINITION: SSD CACHE*/
	typedef struct SSD_cache {
		struct SSD_cache *pre;		//指向前一個CACHE BLOCK
		unsigned long ssd_blkno;	//SSD Block Number
		unsigned long hdd_blkno;	//HDD Block Number
		int dirtyFlag;				//標記是否為Dirty Block
		struct SSD_cache *next;
	} SSD_CACHE;

	/*****With ssd logical partition*****/

	/*USER SSD CACHE LOGICAL PARTITION*/
	static unsigned long userCacheStart[NUM_OF_USER];
	static unsigned long userCacheSize[NUM_OF_USER];
	/*SSD CACHE TABLE*/
	static SSD_CACHE *userCachingTable[NUM_OF_USER];
	static unsigned long userFreeCount[NUM_OF_USER];
	/*CACHING SPACE*/
	//記錄實際的Cache狀態(Dirty, Clean or Free)
	int userCachingSpace[SSD_CACHING_SPACE_BY_BLOCKS];

	/*USER CACHE INITIALIZATION*/
	int initUserCACHE();
	/*INSERT CACHE TABLE BY USER*/
	int insertCACHEByUser(unsigned long *ssd_blk, unsigned long *hdd_blk, int reqFlag, unsigned userno);
	/*CACHE EVICTION POLICY:SPECIFIC HDD Block and User*/
	SSD_CACHE *evictCACHEByUser(unsigned long hdd_blk, unsigned userno);
	/*SEARCH CACHE BY USER*/
	SSD_CACHE *searchCACHEByUser(unsigned long hdd_blk, unsigned userno);
	/*CHECK CACHE FULL BY USER*/
	int isFullCACHEByUser(unsigned userno);
	/*GET FREE CACHE BY USER*/
	unsigned long getFreeCACHEByUser(unsigned userno);
	/*CACHING TABLE OUTPUT*/
	void printCACHEByLRUandUsers();

	/*****Without ssd logical partition*****/

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