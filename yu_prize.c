#include "yu_prize.h"

/*PRINT METADATA BLOCK TABLE*/
/**
 * [印出Metadata Block Table]
 */
void metaTablePrint() {
	METABLOCK *search;
	search = APN;
	printf(COLOR_GB);
	printf("----------------------------------------------------------------------------------------------------\n");
	printf("-[<APN>METADATA BLOCK TABLE]\n");
	while(search != NULL) {
		printf("-    [PRIZE] ssd_blkno =%8lu hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u *prize =%3lf\n", search->ssd_blkno, search->hdd_blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
		search = search->next;
	}

	search = CPN;
	printf("-[<CPN>METADATA BLOCK TABLE]\n");
	while(search != NULL) {
		printf("-    [PRIZE]                     hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u *prize =%3lf\n", search->hdd_blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
		search = search->next;
	}
	printf("----------------------------------------------------------------------------------------------------\n");
	printf(COLOR_N);
}

/*GET PRIZE*/
/**
 * [計算Prize值]
 * @param {unsigned int} readCnt [Block讀取次數]
 * @param {unsigned int} writeCnt [Block寫入次數]
 * @param {unsigned int} seqLen [多少pages(預設block的總page數)]
 * @return {double} - [Prize值]
 */
double getPrize(unsigned int readCnt, unsigned int writeCnt, unsigned int seqLen) {
	return (ALPHA*(((double)readCnt+1)/((double)writeCnt*(double)seqLen+1))+(1-ALPHA)*basePrize);
}

/*UPDATE METADATA BLOCK TABLE */
/**
 * [更新Metadata Block，未取得最新的Prize值]
 * @param {METABLOCK*} metablk [欲更新的Metadata Block]
 * @param {REQ*} tmp [Request]
 */
void metaTableUpdate(METABLOCK *metablk, REQ *tmp) {
	//Modify readCnt and writeCnt
	switch (tmp->reqFlag) {
		case DISKSIM_READ:
			metablk->readCnt++;
			break;
		case DISKSIM_WRITE:
			metablk->writeCnt++;
			break;
		default:
			break;
	}
	//Modify seqLen
	//size為多少個Disksim block(512bytes)，因此轉換為byte再轉換SSD page數
	//if (metablk->seqLen < (tmp->reqSize*DISKSIM_SECTOR)/SSD_PAGE_SIZE)
	//	metablk->seqLen = (tmp->reqSize*DISKSIM_SECTOR)/SSD_PAGE_SIZE;
	//目前以存取整個SSD Block為單位
	metablk->seqLen = SSD_BLOCK_SIZE/SSD_PAGE_SIZE;

	metablk->prize = getPrize(metablk->readCnt, metablk->writeCnt, metablk->seqLen);

	//printf("[PRIZE]metaTableUpdate():METABLOCK ssd_blkno =%8lu hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", metablk->ssd_blkno, metablk->hdd_blkno, metablk->readCnt, metablk->writeCnt, metablk->seqLen, metablk->prize);
}

/*RECORD METADATA BLOCK TABLE */
/**
 * [新增Metadata Block至指定Table]
 * @param {METABLOCK**} metaTable [指向Metadata Block Table指標的指標]
 * @param {REQ} tmp [Request]
 */
void metaTableRecord(METABLOCK **metaTable, REQ *tmp) {
	METABLOCK *search;
	search = (METABLOCK *) calloc(1, sizeof(METABLOCK));
	//將Disksim block(512bytes) number 轉換為 SSD block(SSD_BLOCK_SIZE) number
	search->ssd_blkno = 0;
	search->hdd_blkno = tmp->blkno;
	//printf("%lu\n", search->blkno);
	//Modify readCnt and writeCnt
	switch (tmp->reqFlag) {
		case DISKSIM_READ:
			search->readCnt = 1;
			break;
		case DISKSIM_WRITE:
			search->writeCnt = 1;
			break;
		default:
			break;
	}

	//Modify seqLen
	//tmp->reqSize為多少個Disksim block(512bytes)，因此轉換為byte再轉換SSD page數
	//search->seqLen = (tmp->reqSize*DISKSIM_SECTOR)/SSD_PAGE_SIZE;
	//目前以存取整個SSD Block為單位
	search->seqLen = SSD_BLOCK_SIZE/SSD_PAGE_SIZE;

	search->prize = getPrize(search->readCnt, search->writeCnt, search->seqLen);

	search->next = *metaTable;
	*metaTable = search;
	
	//printf("[PRIZE]metaTableRecord():METABLOCK ssd_blkno =%8lu hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", search->ssd_blkno, search->hdd_blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
}

/*SEARCH METADATA BLOCK TABLE*/
/**
 * [根據指定的Block Number(for HDD)搜尋指定的Metadata BlockTable]
 * @param {METABLOCK*} metaTable [指定的Metadata Block Table]
 * @param {unsigned long} blkno [指定的Block Number(for HDD)]
 * @return {METABLOCK*} search/NULL [搜尋Metadata Block結果]
 */
METABLOCK *metadataSearch(METABLOCK *metaTable, unsigned long blkno) {
	METABLOCK *search = NULL;
	search = metaTable;
	if (metaTable == NULL)
		return NULL;
	
	while(search != NULL) {
		if (search->hdd_blkno == blkno) {
			return search;
		}
		else
			search = search->next;
	}
	return NULL;
}

/*SEARCH METADATA BLOCK TABLE FOR MINIMAL PRIZE*/
/**
 * [於指定的Metadata BlockTable中，搜尋有最小Prize值的Metadata Block]
 * @param {METABLOCK*} metaTable [指定的Metadata Block Table]
 * @return {METABLOCK*} min/NULL [搜尋Metadata Block結果]
 */
METABLOCK *metadataSearchByMinPrize(METABLOCK *metaTable) {
	METABLOCK *search = NULL, *min;
	search = metaTable;
	min = metaTable;
	if (metaTable == NULL)
		return NULL;

	while(search != NULL) {
		if (search->prize <= min->prize) {
			min = search;
		}
		search = search->next;
	}
	//printf("[PRIZE]metadataSearchByMinPrize():Blkno:%lu, Min prize:%lf\n", min->ssd_blkno, min->prize);
	return min;
}

/*CONVERT METADATA BLOCK TABLE*/
/**
 * [將某METADATA BLOCK TABLE的Block移至另一個METADATA BLOCK TABLE]
 * @param {METABLOCK**} oriTable [來源METADATA BLOCK TABLE,雙指標]
 * @param {METABLOCK**} objTable [目的METADATA BLOCK TABLE,雙指標]
 * @param {METABLOCK*} metablk [欲轉移的Metadata Block]
 * @return {int} 0/-1 [Error flag(True or False)]
 */
int metaTableConvert(METABLOCK **oriTable, METABLOCK **objTable, METABLOCK *metablk) {
	//若原Table的第一筆即為欲轉移的block
	if (*oriTable == metablk) {
		*oriTable = metablk->next;
		metablk->next = *objTable;
		*objTable = metablk;
		return 0;
	}
	else {
		METABLOCK *search;
		for (search = *oriTable; search->next != NULL; search=search->next) {
			if(search->next == metablk) {
				search->next = metablk->next;
				metablk->next = *objTable;
				*objTable = metablk;
				return 0;
			}
		}
	}
	return -1;
}

/*PRIZE CACHING*/
/**
 * [根據欲處理的Request，決定系統的快取機制，並產生額外的IO Request]
 * @param @param {REQ} tmp [欲處理的Request]
 * @return {double} response [完成tmp的Response Time(注意!!此時間包含系統Request的Response Time)]
 */
double prizeCaching(REQ *tmp) {
	//PC演算法
	double response = 0;
	int flag = 0;
	if (tmp->reqFlag == DISKSIM_READ) {
		flag = BLOCK_FLAG_CLEAN;
		pcst.UserRReq++;
	}
	else
		flag = BLOCK_FLAG_DIRTY;
	
	//(1)確認是否為APN或CPN，更新meta data(r/w count)
	METABLOCK *search_APN, *search_CPN;
	search_APN = metadataSearch(APN, tmp->blkno);
	search_CPN = metadataSearch(CPN, tmp->blkno);
	//(2)若為APN，則更新APN並發出SSDsim request
	if (search_APN != NULL) {
		pcst.hitCount++;
		//(3a)更新metadata(prize)
		metaTableUpdate(search_APN, tmp);
		//Read,Write SSDsim
		REQ *r;
		r = calloc(1, sizeof(REQ));
		copyReq(tmp, r);
		r->blkno = ssdBlk2simSector(search_APN->ssd_blkno);
		response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
		pcst.totalUserReq++;
		free(r);
	}
	else {
		pcst.missCount++;
		//(2)若為CPN，則更新CPN並考慮轉至APN
		if (search_CPN != NULL) {
			//(3b)更新metadata(prize)
			metaTableUpdate(search_CPN, tmp);
		}
		else {//(2)若無CPN，則新增CPN並考慮轉至APN
			//(3b)新增metadata(prize)
			metaTableRecord(&CPN, tmp);
			//新增的CPN在head端
			search_CPN = CPN;
		}
		//(4b)比較MIN_PRIZE，若大於等於則考慮轉至APN
		if (search_CPN->prize >= MIN_PRIZE) {
			//(5b)考慮Caching space是否滿足所需空間(BLOCK)，若成立則轉至APN
			//Cache or Evict 並且發出相對應的SSDsim & HDDsim requests
			if (insertCACHE(&search_CPN->ssd_blkno, &search_CPN->hdd_blkno, flag) != -1) {
				//printf("[PRIZE]CACHE  METABLOCK ssd_blkno =%8lu hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", search_CPN->ssd_blkno, search_CPN->hdd_blkno, search_CPN->readCnt, search_CPN->writeCnt, search_CPN->seqLen, search_CPN->prize);
				//CPN to APN
				if (metaTableConvert(&CPN, &APN, search_CPN) == -1)
					PrintError(-1, "[PRIZE]metaTableConvert() error(CPN->APN)");
				//如果是Read, 則Read HDDsim & Write SSDsim
				if (tmp->reqFlag == DISKSIM_READ) {
					REQ *r;
					r = calloc(1, sizeof(REQ));
					copyReq(tmp, r);
					//Read HDDsim
					response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
					pcst.totalUserReq++;
					//Write SSDsim
					r->reqFlag = DISKSIM_WRITE;
					r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
					response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
					pcst.totalSysReq++;
					free(r);
				}
				else {//如果是Write, 則Write SSDsim
					REQ *r;
					r = calloc(1, sizeof(REQ));
					copyReq(tmp, r);
					r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
					response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
					pcst.totalUserReq++;
					free(r);
				}
			}
			else {//(6b)比較有最小prize的APN，作為取代進cache的對象 
				METABLOCK *minAPN;
				minAPN = metadataSearchByMinPrize(APN);
				//若欲Cache的CPN的Prize >= The APN with min prize
				if (search_CPN->prize >= minAPN->prize) {
					//(7b)更新Base Prize
					basePrize = minAPN->prize;
					//(8b)更新此Prize
					search_CPN->prize = getPrize(search_CPN->readCnt, search_CPN->writeCnt, search_CPN->seqLen);
					//(9b)剔除Min APN至CPN
					SSD_CACHE *evict;
					evict = evictCACHE(minAPN->hdd_blkno);
					//APN to CPN
					if (metaTableConvert(&APN, &CPN, minAPN) == -1)
						PrintError(-1, "[PRIZE]metaTableConvert() error(APN->CPN)");
					//如果是Dirty, 則Read SSDsim & Write HDDsim; Clean則沒事
					if (evict->dirtyFlag == BLOCK_FLAG_DIRTY) {
						REQ *r1, *r2;
						r1 = calloc(1, sizeof(REQ));
						r2 = calloc(1, sizeof(REQ));
						copyReq(tmp, r1);
						copyReq(tmp, r2);
						r1->blkno = ssdBlk2simSector(evict->ssd_blkno);
						r1->reqFlag = DISKSIM_READ;
						r2->blkno = evict->hdd_blkno;
						r2->reqFlag = DISKSIM_WRITE;
						response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r1);
						pcst.totalSysReq++;
						response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, r2);
						pcst.totalSysReq++;
						free(r1);
						free(r2);
						pcst.dirtyCount++;
					}
					pcst.evictCount++;
					
					//Caching
					if (insertCACHE(&search_CPN->ssd_blkno, &search_CPN->hdd_blkno, flag) != -1) {
						//printf("[PRIZE]CACHE  METABLOCK ssd_blkno =%8lu hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", search_CPN->ssd_blkno, search_CPN->hdd_blkno, search_CPN->readCnt, search_CPN->writeCnt, search_CPN->seqLen, search_CPN->prize);
						//CPN to APN
						if (metaTableConvert(&CPN, &APN, search_CPN) == -1)
							PrintError(-1, "[PRIZE]metaTableConvert() error(CPN->APN)");
						//如果是Read, 則Read HDDsim & Write SSDsim
						if (tmp->reqFlag == DISKSIM_READ) {
							REQ *r;
							r = calloc(1, sizeof(REQ));
							copyReq(tmp, r);
							//Read HDDsim
							response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
							pcst.totalUserReq++;
							//Write SSDsim
							r->reqFlag = DISKSIM_WRITE;
							r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
							response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
							pcst.totalSysReq++;
							free(r);
						}
						else {//如果是Write, 則Write SSDsim
							REQ *r;
							r = calloc(1, sizeof(REQ));
							copyReq(tmp, r);
							r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
							response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
							pcst.totalUserReq++;
							free(r);
						}
					}
					else
						PrintError(-1, "[PRIZE]After eviction, caching error!");
				}
				else {
					//Read,Write HDDsim
					response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
					pcst.totalUserReq++;
				}
			}
		}
		else {
			//Read,Write HDDsim
			response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
			pcst.totalUserReq++;
		}
	}
	//printCACHEByLRU();
	//metaTablePrint();
	return response;
}

/*SEND REQUEST TO SIMULATOR*/
/**
 * [根據Message Queue傳送Request給SSDsim或HDDsim，等待回傳Response time]
 * @param {key_t} key [根據SSDsim或HDDsim的Message Queue之Key值]
 * @param {long} msgtype [指定Message Queue]
 * @param {REQ*} r [欲處理的Request]
 * @return {double} response [Response Time]
 */
double sendRequest(key_t key, long msgtype, REQ *r) {
	if(sendRequestByMSQ(key, r, msgtype) == -1)
        PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");

    pcst.totalBlkReq++;
    if (key == KEY_MSQ_DISKSIM_1)
	    pcst.ssdBlkReq++;

    double response = -1;
    if (key == KEY_MSQ_DISKSIM_1) {
    	REQ *rtn;
    	rtn = calloc(1, sizeof(REQ));
    	if(recvRequestByMSQ(key, rtn, MSG_TYPE_DISKSIM_1_SERVED) == -1)
    	    PrintError(-1, "[PC]A request not received from MSQ in recvRequestByMSQ():");
    	response = rtn->responseTime;
    	//printf("PC_ResponseTime=%lf\n", response);
    	free(rtn);
    	return response;
    }
    else if (key == KEY_MSQ_DISKSIM_2) {
    	REQ *rtn;
    	rtn = calloc(1, sizeof(REQ));
    	if(recvRequestByMSQ(key, rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
    	    PrintError(-1, "[PC]A request not received from MSQ in recvRequestByMSQ():");
    	response = rtn->responseTime;
    	//printf("PC_ResponseTime=%lf\n", response);
    	free(rtn);
    	return response;
    }
    else
    	PrintError(-1, "Send/Receive message with wrong key");
    return response;
}

/**
 * [印出PC Statistic]
 */
void pcStatistic() {
	printf(COLOR_BB"[PRIZE] Total Block Requests(SSD/HDD):%lu(%lu/%lu)\n"COLOR_N, pcst.totalBlkReq, pcst.ssdBlkReq, pcst.totalBlkReq-pcst.ssdBlkReq);
	printf(COLOR_BB"[PRIZE] Total User Requests(R/W):     %lu(%lu/%lu)\n"COLOR_N, pcst.totalUserReq, pcst.UserRReq, pcst.totalUserReq-pcst.UserRReq);
	printf(COLOR_BB"[PRIZE] Total System Requests:        %lu\n"COLOR_N, pcst.totalSysReq);
	printf(COLOR_BB"[PRIZE] Count of Eviction(Dirty):     %lu(%lu)\n"COLOR_N, pcst.evictCount, pcst.dirtyCount);
	printf(COLOR_BB"[PRIZE] Hit rate(Hit/Miss):           %lf(%lu/%lu)\n"COLOR_N, (double)pcst.hitCount/(double)(pcst.hitCount+pcst.missCount), pcst.hitCount, pcst.missCount);
	
}