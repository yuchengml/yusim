#ifndef YU_PRIZE_H
#define YU_PRIZE_H
#include <stdlib.h>
#include "disksim_interface.h" //for flag(DISKSIM_READ) used
#include "yu_structure.h"      //for REQ structure used
#include "yu_parameter.h"
#include "yu_debug.h"
#include "yu_ipc.h"
#include "yu_cache.h"
#include "yu_statistics.h"

	/*系統定義Base Prize*/
    #ifdef YUSIM_LOGICAL_PARTITION
		//Maintain the individual basePrize 
	    static double basePrize[NUM_OF_USER];
    #else
        static double basePrize=0;
    #endif

	/*STRUCTURE DRFINITION: METADATA BLOCK*/
	typedef struct metaBlock {
	    unsigned long blkno;		//以SSD Block size的Block number
	    unsigned readCnt;			//讀取次數
	    unsigned writeCnt;			//寫入次數
	    unsigned seqLen;			//循序存取機會。根據LBPC定義於SSD Block中，計算有多少pages被存取(目前預設一個block的總page數)
	    double prize;				//紀錄Prize值
	    struct metaBlock *next;		//指向下一個Metadata block
	} METABLOCK;

	//Metadata block table
	static METABLOCK *metaTable[NUM_OF_USER];
	static unsigned long metaCnt=0;

	/*STRUCTURE DRFINITION: PC STATISTICS*/
	/**[Record prize caching statistics]
	 * 此紀錄係針對整體系統，無視User個別的紀錄
	 */
	typedef struct pcStat {
		unsigned long totalReq;		//Request數量
		unsigned long ssdReq;		//SSD Request數量
		unsigned long totalUserReq;	//User Request數量
		unsigned long UserRReq;		//User Read Request數量
		unsigned long totalSysReq;	//System Request數量
		unsigned long evictCount;	//Eviction次數
		unsigned long dirtyCount;	//Dirty Block Eviction次數
		unsigned long hitCount;		//Hit次數
		unsigned long missCount;	//Miss次數
		double serviceTime;			//Service time for system
	} PCSTAT;
	
	/*PC STATISTICS*/
	static PCSTAT pcst = {0,0,0,0,0,0,0,0,0,0};

	/*取得Metadata Block 數量*/
	unsigned long getMetaCnt();

	/*GET PRIZE*/
	double getPrize(unsigned int readCnt, unsigned int writeCnt, unsigned int seqLen, unsigned userno);

	/*INITIAL METADATA TABLE*/
	void initmetaTable();

	/*PRINT METADATA BLOCK TABLE*/
	void metaTablePrint();
	/*UPDATE METADATA BLOCK TABLE*/
	void metaTableUpdate(METABLOCK *metablk, REQ *tmp);
	/*RECORD METADATA BLOCK TABLE*/
	METABLOCK *metaTableRecord(REQ *tmp);

	/*****With ssd logical partition*****/
	/*SEARCH METADATA BLOCK TABLE BY USER*/
	METABLOCK *metadataSearchByUser(unsigned long diskBlk, unsigned userno);
	/*SEARCH METADATA BLOCK TABLE FOR USER WITH MINIMAL PRIZE*/
	double metadataSearchByUserWithMinPrize(unsigned userno);

	/*PRIZE CACHING*/
	double prizeCaching(REQ *tmp, double time);
	/*SEND REQUEST TO SIMULATOR*/
	double sendRequest(key_t key, long int msgtype, REQ *r);
	
	/*印出PC Statistics*/
	void pcStatistics();
	/*寫檔至 Result File*/
	void pcWriteResultFile(FILE **result);
#endif