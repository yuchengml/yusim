#include "yu_prize.h"

unsigned long totalBlkReq = 0;
unsigned long evictCount = 0;
unsigned long dirtyCount = 0;
unsigned long ssdBlkReq = 0;

METABLOCK *APN = NULL; //SSD metadata block
METABLOCK *CPN = NULL; //HDD metadata block

double basePrize=0;
double alpha=0.7;

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

double getPrize(unsigned int readCnt, unsigned int writeCnt, unsigned int seqLen) {
	
	return (alpha*(((double)readCnt+1)/((double)writeCnt*(double)seqLen+1))+(1-alpha)*basePrize);

}

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

	//printf("[PRIZE]Update METABLOCK ssd_blkno =%8lu hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", metablk->ssd_blkno, metablk->hdd_blkno, metablk->readCnt, metablk->writeCnt, metablk->seqLen, metablk->prize);
}

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
	
	//printf("[PRIZE]Record METABLOCK ssd_blkno =%8lu hdd_blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", search->ssd_blkno, search->hdd_blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
}

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
	//printf("[PRIZE]Min prize:%lf\n", min->prize);
	return min;
}

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

void prizeCaching(REQ *tmp) {
	//PC演算法
	int flag = 0;
	if (tmp->reqFlag == DISKSIM_READ)
		flag = BLOCK_FLAG_CLEAN;
	else
		flag = BLOCK_FLAG_DIRTY;
	
	//(1)確認是否為APN或CPN，更新meta data(r/w count)
	METABLOCK *search_APN, *search_CPN;
	search_APN = metadataSearch(APN, tmp->blkno);
	search_CPN = metadataSearch(CPN, tmp->blkno);
	//(2)若為APN，則更新APN並發出SSDsim request
	if (search_APN != NULL) {
		//(3a)更新metadata(prize)
		metaTableUpdate(search_APN, tmp);
		//Read,Write SSDsim
		REQ *r;
		r = calloc(1, sizeof(REQ));
		copyReq(tmp, r);
		r->blkno = ssdBlk2simSector(search_APN->ssd_blkno);
		sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
		free(r);
	}
	else {
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
					sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
					//Write SSDsim
					r->reqFlag = DISKSIM_WRITE;
					r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
					sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
					free(r);
				}
				else {//如果是Write, 則Write SSDsim
					REQ *r;
					r = calloc(1, sizeof(REQ));
					copyReq(tmp, r);
					r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
					sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
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
					if (evict->flag == BLOCK_FLAG_DIRTY) {
						REQ *r1, *r2;
						r1 = calloc(1, sizeof(REQ));
						r2 = calloc(1, sizeof(REQ));
						copyReq(tmp, r1);
						copyReq(tmp, r2);
						r1->blkno = evict->ssd_blkno;
						r1->reqFlag = DISKSIM_READ;
						r2->blkno = evict->hdd_blkno;
						r2->reqFlag = DISKSIM_WRITE;
						sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r1);
						sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, r2);
						free(r1);
						free(r2);
						dirtyCount++;
					}
					evictCount++;
					
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
							sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
							//Write SSDsim
							r->reqFlag = DISKSIM_WRITE;
							r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
							sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
							free(r);
						}
						else {//如果是Write, 則Write SSDsim
							REQ *r;
							r = calloc(1, sizeof(REQ));
							copyReq(tmp, r);
							r->blkno = ssdBlk2simSector(search_CPN->ssd_blkno);
							sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
							free(r);
						}
					}
					else
						PrintError(-1, "[PRIZE]Caching error");
				}
				else {
					//Read,Write HDDsim
					sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
				}
			}
		}
		else {
			//Read,Write HDDsim
			sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
		}
	}
	//printCACHEByLRU();
}

void sendRequest(key_t key, long int msgtype, REQ *r) {
	/*
	REQ *r;
	r = calloc(1, sizeof(REQ));
	r->arrivalTime = arrivalTime;
	r->devno = devno;
	r->blkno = blkno;
	r->reqSize = reqSize;
	r->reqFlag = reqFlag;
	r->userno = userno;
	r->responseTime = responseTime;
	*/
	if(sendRequestByMSQ(key, r, msgtype) == -1)
        PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");

    totalBlkReq++;
    if (key == KEY_MSQ_DISKSIM_1)
	    ssdBlkReq++;
}

void pcStatistic() {
	printf(COLOR_BB"[PRIZE] Total Block Requests:        %lu\n"COLOR_N, totalBlkReq);
	printf(COLOR_BB"[PRIZE] Total Block Requests for SSD:%lu\n"COLOR_N, ssdBlkReq);
	printf(COLOR_BB"[PRIZE] Count of Eviction:           %lu\n"COLOR_N, evictCount);
	printf(COLOR_BB"[PRIZE] Count of Dirty:              %lu\n"COLOR_N, dirtyCount);
	
}