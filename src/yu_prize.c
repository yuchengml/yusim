#include "yu_prize.h"

/**
 * [取得Metadata Block 數量]
 * @return {unsigned long} metaCnt [Metadata Block 數量]
 */
unsigned long getMetaCnt() {
    return metaCnt;
}

/*GET PRIZE*/
/**
 * [計算Prize值]
 * @param {unsigned int} readCnt [Block讀取次數]
 * @param {unsigned int} writeCnt [Block寫入次數]
 * @param {unsigned int} seqLen [多少pages]
 * @return {double} - [Prize值]
 */
double getPrize(unsigned int readCnt, unsigned int writeCnt, unsigned int seqLen, unsigned userno) {
    //Calculate and return prize value
    #ifdef YUSIM_LOGICAL_PARTITION
        return (ALPHA*(((double)readCnt+1)/((double)writeCnt*(double)seqLen+1))+(1-ALPHA)*basePrize[userno-1]);
    #else
        return (ALPHA*(((double)readCnt+1)/((double)writeCnt*(double)seqLen+1))+(1-ALPHA)*basePrize);
    #endif
}

/*INITIAL METADATA TABLE*/
/**
 * [初始化 Metadata table]
 */
void initmetaTable() {
    unsigned i; //For loop
    //Initialize metadata table
    for (i = 0; i < NUM_OF_USER; i++) {
        metaTable[i] = NULL;
        #ifdef YUSIM_LOGICAL_PARTITION
            basePrize[i] = 0;
        #else
            basePrize = 0;
        #endif
    }
}

/*PRINT METADATA BLOCK TABLE*/
/**
 * [印出Metadata Block Table]
 */
void metaTablePrint() {
    METABLOCK *search;
    unsigned i;
    printf(COLOR_GB);
    printf("----------------------------------------------------------------------------------------------------\n");
    for (i = 0; i < NUM_OF_USER; i++) {
        search = metaTable[i];
        
        printf("-[USER_%u METADATA BLOCK TABLE]\n", i+1);
        while(search != NULL) {
            printf("-   [PRIZE] blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u *prize =%3lf\n", search->blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
            search = search->next;
        }
        printf("----------------------------------------------------------------------------------------------------\n");
        printf(COLOR_N);
    }
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

    //Update prize
    metablk->prize = getPrize(metablk->readCnt, metablk->writeCnt, metablk->seqLen, tmp->userno);

    //DEBUG:
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
    //New metdata (The unit is block for SSD)
    METABLOCK *search;
    search = (METABLOCK *) calloc(1, sizeof(METABLOCK));

    //Count the number of metadata
    metaCnt++;

    //Record block number in metadata
    //Hint: Change Disksim block(512bytes) number to SSD block(SSD_BLOCK_SIZE) number
    search->blkno = tmp->diskBlkno/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK);

    //Record readCnt and writeCnt in metadata
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

    //Record seqLen in metadata
    search->seqLen = 0;

    //Record prize in metadata
    search->prize = getPrize(search->readCnt, search->writeCnt, search->seqLen, tmp->userno);

    //Insert metadata into metadata table
    #ifdef YUSIM_LOGICAL_PARTITION
        //Maintain the individual metadata tables
        search->next = metaTable[tmp->userno-1];
        metaTable[tmp->userno-1] = search;
    #else
        //Maintain only one metadata table by index:0
        search->next = metaTable[0];
        metaTable[0] = search;
    #endif
    
    //DEBUG:
    //printf("[PRIZE]metaTableRecord():METABLOCK blkno =%8lu readCnt =%6u writeCnt =%6u seqLen =%3u prize =%3lf\n", search->blkno, search->readCnt, search->writeCnt, search->seqLen, search->prize);
    //metaTablePrint();

    //Return this new metadata
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
    //Determine which metadata table used in this function
    //Hint: unum is a substitute for different modes
    #ifdef YUSIM_LOGICAL_PARTITION
        //Individual metadata tables
        unsigned unum = userno;
    #else
        //All metadatas in one table by index:0
        unsigned unum = 1;
    #endif

    //Assign the metadata table which pc searches
    METABLOCK *search = NULL;
    search = metaTable[unum-1];
    if (metaTable[unum-1] == NULL)
        return NULL;
    
    //Search the metadata
    while(search != NULL) {
        if (search->blkno == diskBlk/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK)) {
            return search;
        }
        else
            search = search->next;
    }

    //Metadata not found
    return NULL;
}

/*SEARCH METADATA BLOCK TABLE FOR USER WITH MINIMAL PRIZE*/
/**
 * [於指定的Metadata BlockTable中，搜尋有最小Prize值的Metadata Block]
 * @param {unsigned} userno [User number(1-n)]
 * @return {double} min->prize [回傳min prize, error:-1]
 */
double metadataSearchByUserWithMinPrize(unsigned userno) {
    //Determine which metadata table used in this function
    //Hint: unum is a substitute for different modes
    #ifdef YUSIM_LOGICAL_PARTITION
        //Individual metadata tables
        unsigned unum = userno;
    #else
        //All metadatas in one table by index:0
        unsigned unum = 1;
    #endif

    //Assign the metadata table which pc searches
    METABLOCK *search=NULL;
    search = metaTable[unum-1];
    if (metaTable[unum-1] == NULL)
        return -1;

    //Search the metadata
    //Hint: This metadata must include some pages in cache 
    METABLOCK *min=NULL;    //Record the metadata with the minimal prize
    //Find the first metadata in the table
    while(search != NULL) {
        //This metadata must include one page in cache at least
        if (search->seqLen > 0) {
            min = search;
            break;
        }
        search = search->next;
    }

    //And, Find the metadata with the minimal prize
    while(search != NULL) {
        if (search->prize < min->prize && search->seqLen > 0) {
            min = search;
        }
        search = search->next;
    }

    //DEBUG:
    //metaTablePrint();
    //printf("[PRIZE]metadataSearchByUserWithMinPrize():Blkno:%lu, Min prize:%lf\n", min->blkno, min->prize);
    
    //Return the minimal prize
    return min->prize;
}

/*PRIZE CACHING*/
/**
 * [根據欲處理的Request，決定系統的快取機制，並產生額外的IO Request]
 * @param @param {REQ} tmp [欲處理的Request]
 * @return {double} service [完成tmp的Service Time(注意!!此時間包含系統Request的Service Time)]
 */
double prizeCaching(REQ *tmp, double time) {
    //Prize caching algorithm
    double service = 0;    //Record service time
    int flag = 0;           //The flag of page

    //Check the type of request
    if (tmp->reqFlag == DISKSIM_READ) {
        flag = PAGE_FLAG_CLEAN;

        //Statistics
        pcst.UserRReq++;
        userst[tmp->userno-1].UserRReq++;
        sysst.UserRReq++;
    }
    else
        flag = PAGE_FLAG_DIRTY;
    
    //Check whether the page is in cache
    SSD_CACHE *cache;
    cache = searchCACHEByUser(tmp->diskBlkno, tmp->userno);

    //Cache Hit: Page found in cache
    if (cache != NULL) {
        //Statistics
        pcst.hitCount++;
        userst[tmp->userno-1].hitCount++;
        sysst.hitCount++;

        //New or update metadata(prize)
        METABLOCK *meta;
        meta = metadataSearchByUser(tmp->diskBlkno, tmp->userno);
        if (meta == NULL)
            meta = metaTableRecord(tmp);
        else
            metaTableUpdate(meta, tmp);

        //Caching
        if (insertCACHEByUser(tmp->diskBlkno, flag, tmp->userno, time, meta) == NULL)
            PrintError(-1, "[PRIZE]insertCACHEByUser() error(cache hit but return full)");

        //Generate IO requests
        //Read or write SSDsim
        REQ *r;
        r = calloc(1, sizeof(REQ));
        copyReq(tmp, r);
        r->diskBlkno = ssdPage2simSector(cache->pageno);
        service += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);
        
        //Statistics
        pcst.totalUserReq++;
        userst[tmp->userno-1].totalUserReq++;
        userst[tmp->userno-1].UserReqInPeriod++;
        sysst.totalUserReq++;
        sysst.UserReqInPeriod++;

        //Release request variable
        free(r);
    }
    else { //Cache Miss: Page not found in cache
        //Statistics
        pcst.missCount++;
        userst[tmp->userno-1].missCount++;
        sysst.missCount++;

        //New or update metadata(prize)
        METABLOCK *meta;
        meta = metadataSearchByUser(tmp->diskBlkno, tmp->userno);
        if (meta == NULL)
            meta = metaTableRecord(tmp);
        else
            metaTableUpdate(meta, tmp);

        //Compare MIN_PRIZE and cache or not
        if (meta->prize >= MIN_PRIZE) {
            //Try to insert cache
            cache = insertCACHEByUser(tmp->diskBlkno, flag, tmp->userno, time, meta);
            //Cache not full(no eviction)
            if (cache != NULL) {
                //Record seqLen in metadata table
                meta->seqLen++;

                //Generate IO requests
                //Read Miss: Read HDDsim & Write SSDsim
                if (tmp->reqFlag == DISKSIM_READ) {
                    REQ *r;
                    r = calloc(1, sizeof(REQ));
                    copyReq(tmp, r);

                    //Read HDDsim
                    service += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);
                    //Statistics
                    pcst.totalUserReq++;
                    userst[tmp->userno-1].totalUserReq++;
                    userst[tmp->userno-1].UserReqInPeriod++;
                    sysst.totalUserReq++;
                    sysst.UserReqInPeriod++;

                    //Write SSDsim
                    r->reqFlag = DISKSIM_WRITE;
                    r->diskBlkno = ssdPage2simSector(cache->pageno);
                    service += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);

                    //Statistics
                    pcst.totalSysReq++;
                    userst[tmp->userno-1].totalSysReq++;
                    sysst.totalSysReq++;

                    //Release request variable
                    free(r);
                }
                else {//Write Miss: Write SSDsim
                    REQ *r;
                    r = calloc(1, sizeof(REQ));
                    copyReq(tmp, r);

                    //Write SSDsim
                    r->diskBlkno = ssdPage2simSector(cache->pageno);
                    service += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);

                    //Statistics
                    pcst.totalUserReq++;
                    userst[tmp->userno-1].totalUserReq++;
                    userst[tmp->userno-1].UserReqInPeriod++;
                    sysst.totalUserReq++;
                    sysst.UserReqInPeriod++;

                    //Release request variable
                    free(r);
                }
            }
            else { //Compare the cached page with the minimal prize
                //Find the minimal prize of the cached page
                double minPrize = -1;
                minPrize = metadataSearchByUserWithMinPrize(tmp->userno);
                if (minPrize == -1)
                    PrintError(minPrize, "[PRIZE]Something error:No caching space and no metadata with minPrize!");
                //The prize of new page >= the minimal prize of the cached page
                if (meta->prize >= minPrize) {
                    //Update baseprize
                    #ifdef YUSIM_LOGICAL_PARTITION
                        basePrize[tmp->userno-1] = minPrize;
                    #else
                        basePrize = minPrize;
                    #endif

                    //Update prize
                    meta->prize = getPrize(meta->readCnt, meta->writeCnt, meta->seqLen, tmp->userno);

                    //Find the minimal prize of the cached page again!
                    //Hint: Updated prize may be the minimal prize
                    minPrize = metadataSearchByUserWithMinPrize(tmp->userno);
                    if (minPrize == -1)
                        PrintError(minPrize, "[PRIZE]Something error:No caching space and no metadata with minPrize!");

                    //Evict the victim page with the minimal prize
                    SSD_CACHE *evict;
                    evict = evictCACHEFromLRUWithMinPrizeByUser(minPrize, tmp->userno);
                    if (evict == NULL)
                        PrintError(-1, "[PRIZE]Cache eviction error: Victim not found!:");
                    
                    //Modify metadata
                    if (evict->pcMeta == NULL)
                        PrintError(-1, "[PRIZE]Something error: Meta. of victim not found!:");
                    evict->pcMeta->seqLen--;
                    
                    //Generate IO requests
                    //If victim page is dirty, Read SSDsim & Write HDDsim
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

                        //Read SSDsim
                        service += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r1);

                        //Statistics
                        pcst.totalSysReq++;
                        userst[tmp->userno-1].totalSysReq++;
                        sysst.totalSysReq++;

                        //Write HDDsim
                        service += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, r2);

                        //Statistics
                        pcst.totalSysReq++;
                        userst[tmp->userno-1].totalSysReq++;
                        sysst.totalSysReq++;

                        //Release request variable
                        free(r1);
                        free(r2);

                        //Statistics
                        pcst.dirtyCount++;
                        userst[tmp->userno-1].dirtyCount++;
                        sysst.dirtyCount++;
                    }

                    //Release variable
                    free(evict);

                    //Statistics
                    pcst.evictCount++;
                    userst[tmp->userno-1].evictCount++;
                    sysst.evictCount++;

                    //Caching
                    cache = insertCACHEByUser(tmp->diskBlkno, flag, tmp->userno, time, meta);
                    if (cache != NULL) {
                        //Record seqLen in metadata table
                        meta->seqLen++;

                        //Generate IO requests
                        //Read Miss: Read HDDsim & Write SSDsim
                        if (tmp->reqFlag == DISKSIM_READ) {
                            REQ *r;
                            r = calloc(1, sizeof(REQ));
                            copyReq(tmp, r);

                            //Read HDDsim
                            service += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);

                            //Statistics
                            pcst.totalUserReq++;
                            userst[tmp->userno-1].totalUserReq++;
                            userst[tmp->userno-1].UserReqInPeriod++;
                            sysst.totalUserReq++;
                            sysst.UserReqInPeriod++;

                            //Write SSDsim
                            r->reqFlag = DISKSIM_WRITE;
                            r->diskBlkno = ssdPage2simSector(cache->pageno);
                            service += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);

                            //Statistics
                            pcst.totalSysReq++;
                            userst[tmp->userno-1].totalSysReq++;
                            sysst.totalSysReq++;

                            //Release request variable
                            free(r);
                        }
                        else {//Write Miss: Write SSDsim
                            REQ *r;
                            r = calloc(1, sizeof(REQ));
                            copyReq(tmp, r);
                            r->diskBlkno = ssdPage2simSector(cache->pageno);

                            //Write SSDsim
                            service += sendRequest(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1, r);

                            //Statistics
                            pcst.totalUserReq++;
                            userst[tmp->userno-1].totalUserReq++;
                            userst[tmp->userno-1].UserReqInPeriod++;
                            sysst.totalUserReq++;
                            sysst.UserReqInPeriod++;

                            //Release request variable
                            free(r);
                        }
                    }
                    else
                        PrintError(-1, "[PRIZE]After eviction, caching error!");
                }
                else {
                    //Read or write HDDsim
                    service += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);

                    //Statistics
                    pcst.totalUserReq++;
                    userst[tmp->userno-1].totalUserReq++;
                    userst[tmp->userno-1].UserReqInPeriod++;
                    sysst.totalUserReq++;
                    sysst.UserReqInPeriod++;
                }
            }
        }
        else {
            //Read or write HDDsim
            service += sendRequest(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2, tmp);

            //Statistics
            pcst.totalUserReq++;
            userst[tmp->userno-1].totalUserReq++;
            userst[tmp->userno-1].UserReqInPeriod++;
            sysst.totalUserReq++;
            sysst.UserReqInPeriod++;
        }
    }

    //Statistics
    pcst.serviceTime += service;

    //Return service time
    return service;
}

/*SEND REQUEST TO SIMULATOR*/
/**
 * [根據Message Queue傳送Request給SSDsim或HDDsim，等待回傳Service time]
 * @param {key_t} key [根據SSDsim或HDDsim的Message Queue之Key值]
 * @param {long} msgtype [指定Message Queue]
 * @param {REQ*} r [欲處理的Request]
 * @return {double} service [Service Time]
 */
double sendRequest(key_t key, long msgtype, REQ *r) {
    //Send IO request
    if(sendRequestByMSQ(key, r, msgtype) == -1)
        PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");

    //Statistics
    pcst.totalReq++;
    userst[r->userno-1].totalReq++;
    sysst.totalReq++;
    if (key == KEY_MSQ_DISKSIM_1) {
        pcst.ssdReq++;
        userst[r->userno-1].ssdReq++;
        sysst.ssdReq++;
    }

    double service = -1;    //Record service time

    //For SSDsim
    if (key == KEY_MSQ_DISKSIM_1) {
        REQ *rtn;
        rtn = calloc(1, sizeof(REQ));

        //Receive serviced request
        if(recvRequestByMSQ(key, rtn, MSG_TYPE_DISKSIM_1_SERVED) == -1)
            PrintError(-1, "[PC]A request not received from MSQ in recvRequestByMSQ():");

        //Record service time
        service = rtn->responseTime;

        //Release request variable and return service time
        free(rtn);
        return service;
    }
    else if (key == KEY_MSQ_DISKSIM_2) {//For HDDsim
        REQ *rtn;
        rtn = calloc(1, sizeof(REQ));

        //Receive serviced request
        if(recvRequestByMSQ(key, rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
            PrintError(-1, "[PC]A request not received from MSQ in recvRequestByMSQ():");

        //Record service time
        service = rtn->responseTime;

        //Release request variable and return service time
        free(rtn);
        return service;
    }
    else
        PrintError(-1, "Send/Receive message with wrong key");

    //Return service time
    return service;
}

/**
 * [印出PC Statistic]
 */
void pcStatistics() {
    printf(COLOR_BB"[PRIZE] Total Page Requests(SSD/HDD): %lu(%lu/%lu)\n"COLOR_N, pcst.totalReq, pcst.ssdReq, pcst.totalReq-pcst.ssdReq);
    printf(COLOR_BB"[PRIZE] Total User Requests(R/W):     %lu(%lu/%lu)\n"COLOR_N, pcst.totalUserReq, pcst.UserRReq, pcst.totalUserReq-pcst.UserRReq);
    printf(COLOR_BB"[PRIZE] Total System Requests:        %lu\n"COLOR_N, pcst.totalSysReq);
    printf(COLOR_BB"[PRIZE] Count of Eviction(Dirty):     %lu(%lu)\n"COLOR_N, pcst.evictCount, pcst.dirtyCount);
    printf(COLOR_BB"[PRIZE] Hit rate(Hit/Miss):           %lf(%lu/%lu)\n"COLOR_N, (double)pcst.hitCount/(double)(pcst.hitCount+pcst.missCount), pcst.hitCount, pcst.missCount);
    printf(COLOR_BB"[PRIZE] Avg. Service Time:            %lf\n"COLOR_N, pcst.serviceTime/pcst.totalUserReq);
}

/**
 * [寫檔至 Result File]
 * @param {FILE*} st [寫檔Pointer]
 */
void pcWriteResultFile(FILE **result) {
    fprintf(*result, "[PRIZE] Total Page Requests(SSD/HDD): %lu(%lu/%lu)\n", pcst.totalReq, pcst.ssdReq, pcst.totalReq-pcst.ssdReq);
    fprintf(*result, "[PRIZE] Total User Requests(R/W):     %lu(%lu/%lu)\n", pcst.totalUserReq, pcst.UserRReq, pcst.totalUserReq-pcst.UserRReq);
    fprintf(*result, "[PRIZE] Total System Requests:        %lu\n", pcst.totalSysReq);
    fprintf(*result, "[PRIZE] Count of Eviction(Dirty):     %lu(%lu)\n", pcst.evictCount, pcst.dirtyCount);
    fprintf(*result, "[PRIZE] Hit rate(Hit/Miss):           %lf(%lu/%lu)\n", (double)pcst.hitCount/(double)(pcst.hitCount+pcst.missCount), pcst.hitCount, pcst.missCount);
    fprintf(*result, "[PRIZE] Avg. Service Time:            %lf\n", pcst.serviceTime/pcst.totalUserReq);
}