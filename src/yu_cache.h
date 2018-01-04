#ifndef YU_CACHE_H
#define YU_CACHE_H
#include <stdlib.h>
#include <stdio.h>
#include "yu_parameter.h"
#include "yu_credit.h"
#include "yu_prize.h"
#include "yu_statistics.h"
#include "yu_debug.h"

	/*STRUCTURE DEFINITION: SSD CACHE*/
	typedef struct SSD_cache {
		unsigned long pageno;		//(In cache)SSD Page Number
		unsigned long diskBlkno;	//(In disk) HDD Block Number//Block編號(根據Disksim格式)
		int dirtyFlag;				//標記是否為Dirty page
		int freeFlag;				//標記是否為Free page
		unsigned user;				//User Number for multi users
		double accessTime;			//Cache的存取時間
		struct metaBlock *pcMeta;			//連結PC的Metadata
	} SSD_CACHE;

	/*SSD CACHE TABLE*/
	static SSD_CACHE ssdCache[SSD_CACHING_SPACE_BY_PAGES];

	/*****With ssd logical partition*****/

	/*USER SSD CACHE LOGICAL PARTITION*/
	static unsigned long userCacheStart[NUM_OF_USER];
	static unsigned long userCacheSize[NUM_OF_USER];

	/*USER FREE CACHE COUNT*/
	static unsigned long userFreeCount[NUM_OF_USER];
	static unsigned long userCacheCount[NUM_OF_USER];

	/*USER CACHE INITIALIZATION*/
	int initUserCACHE();
	/*INSERT CACHE TABLE BY USER*/
	SSD_CACHE *insertCACHEByUser(unsigned long diskBlk, int reqFlag, unsigned userno, double time, struct metaBlock *meta);
	/*CACHE EVICTION POLICY:SPECIFIC Block with min prize and User*/
	SSD_CACHE *evictCACHEFromLRUWithMinPrizeByUser(double minPrize, unsigned userno);
	/*SEARCH CACHE BY USER*/
	SSD_CACHE *searchCACHEByUser(unsigned long diskBlk, unsigned userno);
	/*CHECK CACHE FULL BY USER*/
	int isFullCACHEByUser(unsigned userno);
	/*GET FREE CACHE BY USER*/
	unsigned long getFreeCACHEByUser(unsigned userno);
	/*CACHING TABLE OUTPUT*/
	void printCACHEByLRUandUsers();

	/*GET CACHE COUNT*/
	unsigned long getCacheCnt();
	/*寫檔至 Result File*/
	void CACHEWriteResultFile(FILE **result);
	
#endif