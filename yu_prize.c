#include "yu_prize.h"

/*PRINT METADATA BLOCK TABLE*/
/**
 * [印出Metadata Block Table]
 */
void metaTablePrint() {
	METABLOCK *search;
	search = metaTable;
	printf(COLOR_GB);
	printf("----------------------------------------------------------------------------------------------------\n");
	printf("-[METADATA BLOCK TABLE]\n");
	while(search != NULL) {
		printf("-    [PRIZE] blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u *prize =%3lf\n", search->blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
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
 * @param {unsigned int} seqLen [多少pages]
 * @return {double} - [Prize值]
 */
double getPrize(unsigned int readCnt, unsigned int writeCnt, unsigned int seqLen) {
	return (ALPHA*(((double)readCnt+1)/((double)writeCnt*(double)seqLen+1))+(1-ALPHA)*basePrize);
}

/*UPDATE METADATA BLOCK TABLE */
/**
 * [更新Metadata Block，為取得最新的Prize值]
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

	metablk->prize = getPrize(metablk->readCnt, metablk->writeCnt, metablk->seqLen);

	//printf("[PRIZE]metaTableUpdate():METABLOCK blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", metaTable->blkno, metaTable->readCnt, metaTable->writeCnt, metaTable->seqLen, metaTable->prize);
	//metaTablePrint();
}

/*RECORD METADATA BLOCK TABLE */
/**
 * [新增Metadata Block至指定Table]
 * @param {REQ} tmp [Request]
 * @return {METABLOCK*} search/NULL [New Metadata Block]
 */
METABLOCK *metaTableRecord(REQ *tmp) {
	METABLOCK *search;
	search = (METABLOCK *) calloc(1, sizeof(METABLOCK));
	//將Disksim block(512bytes) number 轉換為 SSD block(SSD_BLOCK_SIZE) number
	search->blkno = tmp->diskBlkno/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK); //轉換成以SSD block大小的number
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
	search->seqLen = 0;

	search->prize = getPrize(search->readCnt, search->writeCnt, search->seqLen);

	search->user = tmp->userno;

	search->next = metaTable;
	metaTable = search;
	
	//printf("[PRIZE]metaTableRecord():METABLOCK blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", search->blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
	//metaTablePrint();
	return search;
}

/*SEARCH METADATA BLOCK TABLE BY USER*/
/**
 * [根據指定的Block Number(for HDD)搜尋指定的Metadata BlockTable]
 * @param {unsigned long} diskBlk [指定的Block Number(for HDD)(disksim格式)]
 * @param {unsigned} userno [User number(1-n)]
 * @return {METABLOCK*} search/NULL [搜尋Metadata Block結果]
 */
METABLOCK *metadataSearchByUser(unsigned long diskBlk, unsigned userno) {
	METABLOCK *search = NULL;
	search = metaTable;
	if (metaTable == NULL)
		return NULL;
	
	while(search != NULL) {
		if (search->blkno == diskBlk/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK) && search->user == userno) {
			return search;
		}
		else
			search = search->next;
	}
	return NULL;
}

/*SEARCH METADATA BLOCK TABLE FOR USER WITH MINIMAL PRIZE*/
/**
 * [於指定的Metadata BlockTable中，搜尋有最小Prize值的Metadata Block]
 * @param {unsigned} userno [User number(1-n)]
 * @return {METABLOCK *} min [搜尋結果metadata block 必須是有cached page的]
 */
METABLOCK *metadataSearchByUserWithMinPrize(unsigned userno) {
	METABLOCK *search=NULL, *min=NULL;
	search = metaTable;
	if (metaTable == NULL)
		return min;

	unsigned i;
	//先找出第一個同User Number的Metadata
	while(search != NULL) {
		if (search->user == userno && search->seqLen > 0) {
			min = search;
			break;
		}
		search = search->next;
	}

	//再比較所有同User Number的Metadata，取帶有最小Prize值的User
	while(search != NULL) {
		if (search->prize < min->prize && search->user == userno && search->seqLen > 0) {
			min = search;
		}
		search = search->next;
	}
	//metaTablePrint();
	//printf("[PRIZE]metadataSearchByUserWithMinPrize():Blkno:%lu, Min prize:%lf\n", min->blkno, min->prize);
	return min;
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
		flag = PAGE_FLAG_CLEAN;
		pcst.UserRReq++;
		userst[tmp->userno-1].UserRReq++;
	}
	else
		flag = PAGE_FLAG_DIRTY;
	
	//檢查是否在Cache
	SSD_CACHE *cache;
	cache = searchCACHEByUser(tmp->diskBlkno, tmp->userno);
	if (cache != NULL) {
		pcst.hitCount++;
		userst[tmp->userno-1].hitCount++;

		//Caching
		if (insertCACHEByUser(&tmp->diskBlkno, flag, tmp->userno) == -1)
			PrintError(-1, "[PRIZE]insertCACHEByUser() error(cache hit but return full)");
		
		//更新metadata(prize)
		METABLOCK *meta;
		meta = metadataSearchByUser(tmp->diskBlkno, tmp->userno);
		if (meta == NULL) {
			PrintError(-1, "[PRIZE]metadataSearchByUser() error(in cache but no metadata)");
		}
		metaTableUpdate(meta, tmp);

		//Read,Write SSDsim
		REQ *r;
		r = calloc(1, sizeof(REQ));
		copyReq(tmp, r);
		r->diskBlkno = ssdPage2simSector(cache->pageno);
		response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
		pcst.totalUserReq++;
		userst[tmp->userno-1].totalUserReq++;
		free(r);
	}
	else { //Cache Miss
		pcst.missCount++;
		userst[tmp->userno-1].missCount++;

		//新增或更新metadata(prize)
		METABLOCK *meta;
		meta = metadataSearchByUser(tmp->diskBlkno, tmp->userno);
		if (meta == NULL)
			meta = metaTableRecord(tmp);
		else
			metaTableUpdate(meta, tmp);

		//比較MIN_PRIZE，大於等於則考慮Caching
		if (meta->prize >= MIN_PRIZE) {
			//Cache size足夠，無須Eviction
			if (insertCACHEByUser(&tmp->diskBlkno, flag, tmp->userno) != -1) {
				cache = searchCACHEByUser(tmp->diskBlkno, tmp->userno);
				if (cache == NULL)
					PrintError(-1, "[PRIZE]insertCACHEByUser() insert error(meta->prize >= MIN_PRIZE):");
				meta->seqLen++;
				//如果是Read, 則Read HDDsim & Write SSDsim
				if (tmp->reqFlag == DISKSIM_READ) {
					REQ *r;
					r = calloc(1, sizeof(REQ));
					copyReq(tmp, r);
					//Read HDDsim
					response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
					pcst.totalUserReq++;
					userst[tmp->userno-1].totalUserReq++;
					//Write SSDsim
					r->reqFlag = DISKSIM_WRITE;
					r->diskBlkno = ssdPage2simSector(cache->pageno);
					response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
					pcst.totalSysReq++;
					userst[tmp->userno-1].totalSysReq++;
					free(r);
				}
				else {//如果是Write, 則Write SSDsim
					REQ *r;
					r = calloc(1, sizeof(REQ));
					copyReq(tmp, r);
					r->diskBlkno = ssdPage2simSector(cache->pageno);
					response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
					pcst.totalUserReq++;
					userst[tmp->userno-1].totalUserReq++;
					free(r);
				}
			}
			else { //比較有最小prize的cached page，作為取代進cache的對象
				METABLOCK *min_meta=NULL;
				min_meta = metadataSearchByUserWithMinPrize(tmp->userno);
				if (min_meta == NULL)
					PrintError(-1, "[PRIZE]Something error:No caching space and no metadata with minPrize!");
				//若欲進入Cache的new page其meta的Prize >= 所有meta中最小的Prize
				if (meta->prize >= min_meta->prize) {
					//更新Base Prize
					basePrize = min_meta->prize;
					//更新此Prize
					meta->prize = getPrize(meta->readCnt, meta->writeCnt, meta->seqLen);
					//剔除page with min Prize
					SSD_CACHE *evict;
					evict = evictCACHEFromLRUByUser(min_meta->blkno, tmp->userno);
					min_meta->seqLen--;
					if (evict == NULL)
						PrintError(-1, "[PRIZE]Cache eviction error:! Victim not found!:");
					//如果Victim page是Dirty, 則Read SSDsim & Write HDDsim; Clean則沒事
					if (evict->dirtyFlag == PAGE_FLAG_DIRTY) {
						REQ *r1, *r2;
						r1 = calloc(1, sizeof(REQ));
						r2 = calloc(1, sizeof(REQ));
						copyReq(tmp, r1);
						copyReq(tmp, r2);
						r1->diskBlkno = ssdPage2simSector(evict->pageno);
						r1->reqFlag = DISKSIM_READ;
						r2->diskBlkno = evict->diskBlkno;
						r2->reqFlag = DISKSIM_WRITE;
						response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r1);
						pcst.totalSysReq++;
						userst[tmp->userno-1].totalSysReq++;
						response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, r2);
						pcst.totalSysReq++;
						userst[tmp->userno-1].totalSysReq++;
						free(r1);
						free(r2);
						pcst.dirtyCount++;
						userst[tmp->userno-1].dirtyCount++;
					}
					free(evict);
					pcst.evictCount++;
					userst[tmp->userno-1].evictCount++;

					//Caching
					if (insertCACHEByUser(&tmp->diskBlkno, flag, tmp->userno) != -1) {
						cache = searchCACHEByUser(tmp->diskBlkno, tmp->userno);
						if (cache == NULL)
							PrintError(-1, "[PRIZE]insertCACHEByUser() insert error(cache after evict):");
						meta->seqLen++;
						//如果是Read, 則Read HDDsim & Write SSDsim
						if (tmp->reqFlag == DISKSIM_READ) {
							REQ *r;
							r = calloc(1, sizeof(REQ));
							copyReq(tmp, r);
							//Read HDDsim
							response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
							pcst.totalUserReq++;
							userst[tmp->userno-1].totalUserReq++;
							//Write SSDsim
							r->reqFlag = DISKSIM_WRITE;
							r->diskBlkno = ssdPage2simSector(cache->pageno);
							response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
							pcst.totalSysReq++;
							userst[tmp->userno-1].totalSysReq++;
							free(r);
						}
						else {//如果是Write, 則Write SSDsim
							REQ *r;
							r = calloc(1, sizeof(REQ));
							copyReq(tmp, r);
							r->diskBlkno = ssdPage2simSector(cache->pageno);
							response += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
							pcst.totalUserReq++;
							userst[tmp->userno-1].totalUserReq++;
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
					userst[tmp->userno-1].totalUserReq++;
				}
			}
		}
		else {
			//Read,Write HDDsim
			response += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
			pcst.totalUserReq++;
			userst[tmp->userno-1].totalUserReq++;
		}
	}

	//printCACHEByLRUandUsers();
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
    userst[r->userno-1].totalBlkReq++;
    if (key == KEY_MSQ_DISKSIM_1) {
	    pcst.ssdBlkReq++;
	    userst[r->userno-1].ssdBlkReq++;
    }

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
void pcStatistics() {
	printf(COLOR_BB"[PRIZE] Total Page Requests(SSD/HDD):%lu(%lu/%lu)\n"COLOR_N, pcst.totalBlkReq, pcst.ssdBlkReq, pcst.totalBlkReq-pcst.ssdBlkReq);
	printf(COLOR_BB"[PRIZE] Total User Requests(R/W):     %lu(%lu/%lu)\n"COLOR_N, pcst.totalUserReq, pcst.UserRReq, pcst.totalUserReq-pcst.UserRReq);
	printf(COLOR_BB"[PRIZE] Total System Requests:        %lu\n"COLOR_N, pcst.totalSysReq);
	printf(COLOR_BB"[PRIZE] Count of Eviction(Dirty):     %lu(%lu)\n"COLOR_N, pcst.evictCount, pcst.dirtyCount);
	printf(COLOR_BB"[PRIZE] Hit rate(Hit/Miss):           %lf(%lu/%lu)\n"COLOR_N, (double)pcst.hitCount/(double)(pcst.hitCount+pcst.missCount), pcst.hitCount, pcst.missCount);	
}
