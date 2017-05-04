#ifndef YU_PRIZE_H
#define YU_PRIZE_H
#include <stdlib.h>
#include "disksim_interface.h" //for flag(DISKSIM_READ) used
#include "yu_structure.h"      //for REQ structure used
#include "yu_parameter.h"
#include "yu_debug.h"
#include "yu_ipc.h"
#include "yu_cache.h"

	/*系統定義Base Prize*/
	static double basePrize=0;

	/*STRUCTURE DRFINITION: METADATA BLOCK*/
	typedef struct metaBlock {
	    unsigned long ssd_blkno;	//以SSD的Block為單位(注意!!有別於DISKSIM的Block大小512bytes)
	    unsigned long hdd_blkno;	//以SSD的Block為單位(注意!!有別於DISKSIM的Block大小512bytes)
	    unsigned readCnt;			//讀取次數
	    unsigned writeCnt;			//寫入次數
	    unsigned seqLen;			//根據LBPC定義於SSD Block中，計算有多少pages被存取(目前預設一個block的總page數)
	    double prize;				//紀錄Prize值
	    struct metaBlock *next;		//指向下一個Metadata block
	} METABLOCK;

	//SSD metadata block
	static METABLOCK *APN = NULL;
	//HDD metadata block
	static METABLOCK *CPN = NULL;

	/*STRUCTURE DRFINITION: PC STATISTIC*/
	/**[Record prize caching statistic]
	 * 此紀錄係針對整體系統，無視User個別的紀錄
	 */
	typedef struct pcStat {
		unsigned long totalBlkReq;	//Request數量
		unsigned long ssdBlkReq;	//SSD Request數量
		unsigned long totalUserReq;	//User Request數量
		unsigned long UserRReq;		//User Read Request數量
		unsigned long totalSysReq;	//System Request數量
		unsigned long evictCount;	//Eviction次數
		unsigned long dirtyCount;	//Dirty Block Eviction次數
		unsigned long hitCount;		//Hit次數
		unsigned long missCount;	//Miss次數
	} PCSTAT;
	
	/*PC STATISTIC*/
	static PCSTAT pcst = {0,0,0,0,0,0,0,0,0};

	/*GET PRIZE*/
	double getPrize(unsigned int readCnt, unsigned int writeCnt, unsigned int seqLen);
	
	/*PRINT METADATA BLOCK TABLE*/
	void metaTablePrint();
	/*UPDATE METADATA BLOCK TABLE */
	void metaTableUpdate(METABLOCK *metablk, REQ *tmp);
	/*RECORD METADATA BLOCK TABLE */
	void metaTableRecord(METABLOCK **metaTable, REQ *tmp);
	/*SEARCH METADATA BLOCK TABLE*/
	METABLOCK *metadataSearch(METABLOCK *metaTable, unsigned long blkno);
	/*SEARCH METADATA BLOCK TABLE FOR MINIMAL PRIZE*/
	METABLOCK *metadataSearchByMinPrize(METABLOCK *metaTable);
	/*CONVERT METADATA BLOCK TABLE*/
	int metaTableConvert(METABLOCK **oriTable, METABLOCK **objTable, METABLOCK *metablk);
	/*PRIZE CACHING*/
	double prizeCaching(REQ *tmp);
	/*SEND REQUEST TO SIMULATOR*/
	double sendRequest(key_t key, long int msgtype, REQ *r);
	
	/*印出PC Statistic*/
	void pcStatistic();
#endif