#include "yu_cache.h"
/**
 * 此檔案為維護本系統之Caching機制，根據Hybrid Storage System設定，將SSD視為HDD之Cache，
 * 因此，Caching Table須同時記錄Data(Block)位於SSD的位址(ssd_blk)和HDD的位址(diskBlk)，以
 * 以確保Caching機制的正確性。
 */

/*****With ssd logical partition*****/

/*USER CACHE INITIALIZATION*/
/**
 * [針對Trace指定不同User權重所占用的空間]
 * @return {int} 0/-1 []
 */
int initUserCACHE() {
    totalWeight = 0;
    unsigned i;
    for (i = 0; i < NUM_OF_USER; i++) {
        totalWeight += userWeight[i];
    }

    if (totalWeight == 0)
        PrintError(0, "[USER CACHE] Total User Weight = ");

    #ifdef YUSIM_LOGICAL_PARTITION
        unsigned long startPage = 0;
        for (i = 0; i < NUM_OF_USER; i++) {
            userCacheStart[i] = startPage;
            userCacheSize[i] = userWeight[i]*SSD_CACHING_SPACE_BY_PAGES/totalWeight;
            userFreeCount[i] = userCacheSize[i];
            startPage += userCacheSize[i];
        }
        
        printf(COLOR_GB" [USER CACHE] Total User Weight:%u, Total Cache Size(Pages):%u\n"COLOR_N, totalWeight, SSD_CACHING_SPACE_BY_PAGES);

        for (i = 0; i < NUM_OF_USER; i++) {
            printf(COLOR_GB" [USER CACHE] User%u: Weight:%u Start(Pages):%lu, Size(Pages):%lu\n"COLOR_N, i+1, userWeight[i], userCacheStart[i], userCacheSize[i]);
        }
    #else
        userCacheStart[0] = 0;
        userCacheSize[0] = SSD_CACHING_SPACE_BY_PAGES;
        userFreeCount[0] = SSD_CACHING_SPACE_BY_PAGES;
        printf(COLOR_GB" [USER CACHE] Total User Weight:%u, Total Cache Size(Pages):%u\n"COLOR_N, totalWeight, SSD_CACHING_SPACE_BY_PAGES);
        printf(COLOR_GB" [USER CACHE] \"No Logical Partition:\" Start(Pages):%lu, Size(Pages):%lu\n"COLOR_N, userCacheStart[0], userCacheSize[0]);
        
    #endif

    for (i = 0; i < NUM_OF_USER; i++) {
        userCacheCount[i] = 0;
    }
    
    for (i = 0; i < SSD_CACHING_SPACE_BY_PAGES; i++) {
        ssdCache[i].pageno = i;
    }

    return 0;
}

/*INSERT CACHE TABLE BY USER*/
/**
 * [針對指定User插入(新增)Caching Block至 User Caching Table]
 * @param {unsigned long} diskBlk [Block Number in HDD]
 * @param {int} reqFlag [此次Caching是否修改Caching Block(參考PAGE_FLAG_XXXX)]
 * @param {unsigned} userno [User number(1-n)]
 * @param {double} time [Cache access time]
 * @return {int} 0/-1 [FULL(-1) or not(0)]
 */
SSD_CACHE *insertCACHEByUser(unsigned long diskBlk, int reqFlag, unsigned userno, double time, struct metaBlock *meta) {
    #ifdef YUSIM_LOGICAL_PARTITION
        unsigned unum = userno;
    #else
        unsigned unum = 1;
    #endif
    SSD_CACHE *search;
    search = searchCACHEByUser(diskBlk, unum);

    //新增一筆資料
    if (search == NULL) {
        unsigned long freePage;
        if (isFullCACHEByUser(unum)){
            return NULL;//FULL
        }
        else
            freePage = getFreeCACHEByUser(unum);

        //ssdCache[freePage].pageno = freePage;
        ssdCache[freePage].diskBlkno = diskBlk;
        ssdCache[freePage].dirtyFlag = reqFlag;
        ssdCache[freePage].freeFlag = PAGE_FLAG_NOT_FREE;
        ssdCache[freePage].user = userno;
        ssdCache[freePage].accessTime = time;
        ssdCache[freePage].pcMeta = meta;

        userFreeCount[unum-1]--;
        userCacheCount[userno-1]++;

        //printf("cache head %lu\n", freePage);
        //Update user statistics:caching space
        userst[userno-1].cachingSpace = (double)(userCacheCount[userno-1])/(double)(SSD_CACHING_SPACE_BY_PAGES);
        return &ssdCache[freePage];
    }
    else {//更新一筆資料
        //更新此次存取的Block flag
        switch (reqFlag) {
            case PAGE_FLAG_CLEAN:
                //Dirty still dirty...
                break;
            case PAGE_FLAG_DIRTY:
                ssdCache[search->pageno].dirtyFlag = reqFlag;
                break;
            default:
                PrintError(reqFlag, "[USER CACHE]Caching Error with unknown flag:");
                break;
        }

        ssdCache[search->pageno].accessTime = time;

        //Update user statistics:caching space
        userst[userno-1].cachingSpace = (double)(userCacheCount[userno-1])/(double)(SSD_CACHING_SPACE_BY_PAGES);
        return &ssdCache[search->pageno];
    }
    
}

/*CACHE EVICTION POLICY:SPECIFIC Block with min prize and User*/
/**
 * [根據指定的HDD Block Number和User Number進行剔除]
 * @param {double} minPrize [指定欲剔除的page是具有最小Prize的metadata Block]
 * @param {unsigned} userno [User number(1-n)]
 * @return {SSD_CACHE*} &ssdCache/NULL [回傳欲剔除的cached page;NULL:未找到]
 */
SSD_CACHE *evictCACHEFromLRUWithMinPrizeByUser(double minPrize, unsigned userno) {
    #ifdef YUSIM_LOGICAL_PARTITION
        unsigned unum = userno;
    #else
        unsigned unum = 1;
    #endif

    //printf("start evicting...\n");
    //printCACHEByLRUandUsers();
    METABLOCK *min_meta = NULL;
    unsigned long pageno, minPriPageno;
    int state=0;//Found:1

    for (pageno = userCacheStart[unum-1]; pageno < userCacheStart[unum-1]+userCacheSize[unum-1]; pageno++) {
        if (ssdCache[pageno].freeFlag == PAGE_FLAG_NOT_FREE) {
            if (ssdCache[pageno].pcMeta->prize == minPrize) {
                minPriPageno = pageno;
                state = 1;
                break;
            }
        }
    }
    
    for (pageno = userCacheStart[unum-1]; pageno < userCacheStart[unum-1]+userCacheSize[unum-1]; pageno++) {
        if (ssdCache[pageno].freeFlag == PAGE_FLAG_NOT_FREE && ssdCache[pageno].accessTime < ssdCache[minPriPageno].accessTime) {
            if (ssdCache[pageno].pcMeta->prize == minPrize) {
                minPriPageno = pageno;
                state = 1;
            }
        }
    }
    //printf("finish evicting...\n");
    if (state == 0) {
        return NULL;
    }
    else {
        ssdCache[minPriPageno].freeFlag = PAGE_FLAG_FREE;
        userFreeCount[unum-1]++;
        userCacheCount[ssdCache[minPriPageno].user-1]--;
        userst[ssdCache[minPriPageno].user-1].cachingSpace = (double)(userCacheCount[ssdCache[minPriPageno].user-1])/(double)(SSD_CACHING_SPACE_BY_PAGES);
        return &ssdCache[minPriPageno];
    }
}

/*SEARCH CACHE BY USER*/
/**
 * [指定HDD Block(disksim格式)和User進行搜尋]
 * @param {unsigned long} diskBlk [指定欲搜尋的Block(disksim格式)]
 * @param {unsigned} userno [User number(1-n)]
 * @return {SSD_CACHE*} &ssdCache/NULL [回傳欲搜尋的cached page;NULL:未找到]
 */
SSD_CACHE *searchCACHEByUser(unsigned long diskBlk, unsigned userno) {
    #ifdef YUSIM_LOGICAL_PARTITION
        unsigned unum = userno;
    #else
        unsigned unum = 1;
    #endif
    unsigned long pageno;
    for (pageno = userCacheStart[unum-1]; pageno < userCacheStart[unum-1]+userCacheSize[unum-1]; pageno++) {
        if (ssdCache[pageno].diskBlkno == diskBlk && ssdCache[pageno].freeFlag == PAGE_FLAG_NOT_FREE) {
            return &ssdCache[pageno];
        }
    }
    
    return NULL;
}


/*CHECK CACHE FULL BY USER*/
/**
 * [根據指定的User檢查 User Cache是否已滿]
 * @param {unsigned} userno [User number(1-n)]
 * @return CACHE_FULL/CACHE_NOT_FULL [Full:1; Not full:0]
 */
int isFullCACHEByUser(unsigned userno) {
    #ifdef YUSIM_LOGICAL_PARTITION
        unsigned unum = userno;
    #else
        unsigned unum = 1;
    #endif

    if (userFreeCount[unum-1] == 0)
        return CACHE_FULL;
    else
        return CACHE_NOT_FULL;
}

/*GET FREE CACHE BY USER*/
/**
 * [根據User Number取得其User Caching Table中一個Free Page]
 * [注意!!欲取得Free Page前必須注意已檢查是否已滿? 建議流程isFullCACHE()->getFreeCACHE()]
 * @param {unsigned} userno [User number(1-n)]
 * @return {unsigned long} pageno [Page Number]
 */
unsigned long getFreeCACHEByUser(unsigned userno) {
    #ifdef YUSIM_LOGICAL_PARTITION
        unsigned unum = userno;
    #else
        unsigned unum = 1;
    #endif
        
    unsigned long pageno;
    for (pageno = userCacheStart[unum-1]; pageno < userCacheStart[unum-1]+userCacheSize[unum-1]; pageno++) {
        if (ssdCache[pageno].freeFlag == PAGE_FLAG_FREE) {
            //printf("Free Page number:%lu\n", pageno);
            return pageno;
        }
    }
    PrintError(-1, "[USER CACHE]Get an invalid free Page number in User Cache");
    //If return this means "NO FREE PAGE"!
    return pageno;
}

/*CACHING TABLE OUTPUT*/
/**
 * [印出User Caching Table]
 */
void printCACHEByLRUandUsers() {
    unsigned i;
    unsigned long pageno;
    for (i = 0; i < NUM_OF_USER; i++) {
        printf("[USER CACHE %u]--", i+1);
        
        for (pageno = userCacheStart[i]; pageno < userCacheStart[i]+userCacheSize[i]; pageno++) {
            if (ssdCache[pageno].freeFlag == PAGE_FLAG_NOT_FREE) {
                printf("%lu:%lu[%lu](%lf) <-> ", ssdCache[pageno].pageno, ssdCache[pageno].diskBlkno, ssdCache[pageno].diskBlkno/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK), ssdCache[pageno].accessTime);;
            }
        }

        printf("NULL (%lu)\n", userCacheSize[i]-userFreeCount[i]);
    }
}

/*GET CACHE COUNT*/
unsigned long getCacheCnt() {
    unsigned i;
    unsigned long cnt=0;
    for (i = 0; i < NUM_OF_USER; i++) {
        cnt += (userCacheSize[i]-userFreeCount[i]);
    }
    return cnt;
}

/**
 * [寫檔至 Result File]
 * @param {FILE*} st [寫檔Pointer]
 */
void CACHEWriteResultFile(FILE **result) {
    unsigned i;
    fprintf(*result, "[USER CACHE] Total User Weight:%u, Total Cache Size(Pages):%u\n", totalWeight, SSD_CACHING_SPACE_BY_PAGES);
    #ifdef YUSIM_LOGICAL_PARTITION
        for (i = 0; i < NUM_OF_USER; i++) {
            fprintf(*result, "[USER CACHE] User%u: Weight:%u Start(Pages):%lu, Size(Pages):%lu\n", i+1, userWeight[i], userCacheStart[i], userCacheSize[i]);
        }
    #else
        fprintf(*result, "[USER CACHE] \"No Logical Partition:\" Start(Pages):%lu, Size(Pages):%lu\n", userCacheStart[0], userCacheSize[0]);
    #endif
}