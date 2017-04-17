#ifndef YU_PRIZE_H
#define YU_PRIZE_H
#include <stdlib.h>
#include "disksim_interface.h" //for flag(DISKSIM_READ) used
#include "yu_structure.h"      //for REQ structure used
#include "yu_parameter.h"
#include "yu_debug.h"
#include "yu_ipc.h"
#include "yu_cache.h"

	// Record program counter
	typedef struct metaBlock {
	    unsigned long ssd_blkno; //以SSD的Block為單位(注意!!有別於DISKSIM的Block大小512bytes)
	    unsigned long hdd_blkno; //以SSD的Block為單位(注意!!有別於DISKSIM的Block大小512bytes)
	    unsigned readCnt;
	    unsigned writeCnt;
	    unsigned seqLen; //根據改良PC的LBPC定義，以一個SSD的Block內，計算有多少pages被存取
	    double prize;
	    struct metaBlock *next;
	} METABLOCK;

	double getPrize(unsigned int readCnt, unsigned int writeCnt, unsigned int seqLen);
	
	void metaTablePrint();
	//void CPNRecord(METABLOCK *metaTable, REQ *tmp);
	void metaTableUpdate(METABLOCK *metablk, REQ *tmp);
	void metaTableRecord(METABLOCK **metaTable, REQ *tmp);
	METABLOCK *metadataSearch(METABLOCK *metaTable, unsigned long blkno);
	METABLOCK *metadataSearchByMinPrize(METABLOCK *metaTable);
	int metaTableConvert(METABLOCK **oriTable, METABLOCK **objTable, METABLOCK *metablk);
	void prizeCaching(REQ *tmp);

	void sendRequest(key_t key, long int msgtype, REQ *r);
	void pcStatistic();
#endif