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

	unsigned long startPage = 0;
	for (i = 0; i < NUM_OF_USER; i++) {
		userCacheStart[i] = startPage;
		userCacheSize[i] = userWeight[i]*SSD_CACHING_SPACE_BY_PAGES/totalWeight;
		userFreeCount[i] = userCacheSize[i];
		startPage += userCacheSize[i];
	}
	
	printf(COLOR_GB" [USER CACHE] Total User Weight:%u, Total Cache Size(Pages):%u\n"COLOR_N, totalWeight, SSD_CACHING_SPACE_BY_PAGES);

	for (i = 0; i < NUM_OF_USER; i++) {
		printf(COLOR_GB" [USER CACHE] User%u: Weight:%u Start(Pages):%lu, Size(Pages):%lu\n"COLOR_N, i, userWeight[i], userCacheStart[i], userCacheSize[i]);
	}

	return 0;
}

/*INSERT CACHE TABLE BY USER*/
/**
 * [針對指定User插入(新增)Caching Block至 User Caching Table]
 * @param {unsigned long*} diskBlk [Block Number in HDD]
 * @param {int} reqFlag [此次Caching是否修改Caching Block(參考PAGE_FLAG_XXXX)]
 * @param {unsigned} userno [User number(1-n)]
 * @return {int} 0/-1 [FULL(-1) or not(0)]
 */
int insertCACHEByUser(unsigned long *diskBlk, int reqFlag, unsigned userno) {
	//新增Caching Table中第一筆資料(Table為空)
	if (userCachingTable[userno-1] == NULL) {
		unsigned long freePage;
		if (isFullCACHEByUser(userno))
			return -1;//FULL
		else
			freePage = getFreeCACHEByUser(userno);

		userCachingTable[userno-1] = calloc(1, sizeof(SSD_CACHE));
		userCachingTable[userno-1]->diskBlkno = *diskBlk;
		userCachingTable[userno-1]->dirtyFlag = reqFlag;
		userCachingTable[userno-1]->user = userno;
		userCachingTable[userno-1]->pre = NULL;
		userCachingTable[userno-1]->next = NULL;
		
		userCachingTable[userno-1]->pageno = freePage;
		userCachingSpace[freePage] = reqFlag;
		userFreeCount[userno-1]--;
		//printf("cache first %lu\n", freePage);
	}
	else {//新增或更新Caching Table中某筆資料
		SSD_CACHE *search;
		search = searchCACHEByUser(*diskBlk, userno);

		//新增一筆資料
		if (search == NULL) {
			unsigned long freePage;
			if (isFullCACHEByUser(userno)){
				return -1;//FULL
			}
			else
				freePage = getFreeCACHEByUser(userno);

			SSD_CACHE *tmp;
			tmp = calloc(1, sizeof(SSD_CACHE));
			tmp->diskBlkno = *diskBlk;
			tmp->dirtyFlag = reqFlag;
			tmp->user = userno;

			userCachingTable[userno-1]->pre = tmp;
			tmp->next = userCachingTable[userno-1];
			tmp->pre = NULL;
			userCachingTable[userno-1] = userCachingTable[userno-1]->pre;

			tmp->pageno = freePage;
			userCachingSpace[freePage] = reqFlag;
			userFreeCount[userno-1]--;
			//printf("cache head %lu\n", freePage);
		}
		else {//更新一筆資料
			//更新此次存取的Block flag
			switch (reqFlag) {
				case PAGE_FLAG_CLEAN:
					//Dirty still dirty...
					break;
				case PAGE_FLAG_DIRTY:
					userCachingSpace[search->pageno] = reqFlag;
					break;
				default:
					PrintError(reqFlag, "[USER CACHE]Caching Error with unknown flag:");
					break;
			}

			//實現LRU順序，係為方便印出結果
			//欲cache的block不位於MRU端才需調整
			if (search->pre != NULL) {
				//此次存取的Block移至MRU端；
				if (search->next == NULL) {
					search->pre->next = NULL;
					userCachingTable[userno-1]->pre = search;
					search->next = userCachingTable[userno-1];
					search->pre = NULL;
					userCachingTable[userno-1] = userCachingTable[userno-1]->pre;
				}
				else {
					search->pre->next = search->next;
					search->next->pre = search->pre;
					userCachingTable[userno-1]->pre = search;
					search->next = userCachingTable[userno-1];
					search->pre = NULL;
					userCachingTable[userno-1] = userCachingTable[userno-1]->pre;
				}
			}
		}
	}
	//Update user statistics:caching space
	userst[userno-1].cachingSpace = (double)(userCacheSize[userno-1] - userFreeCount[userno-1])/(double)(SSD_CACHING_SPACE_BY_PAGES);
	//printCACHEByLRUandUsers();
	return 0;
}

/*CACHE EVICTION POLICY:SPECIFIC Block with min prize and User*/
/**
 * [根據指定的HDD Block Number和User Number進行剔除]
 * @param {double} minPrize [指定欲剔除的page是具有最小Prize的metadata Block]
 * @param {unsigned} userno [User number(1-n)]
 * @return {SSD_CACHE*} search/NULL [回傳欲剔除的Block Pointer;NULL:未找到]
 */
SSD_CACHE *evictCACHEFromLRUWithMinPrizeByUser(double minPrize, unsigned userno) {
	//printCACHEByLRUandUsers();
	SSD_CACHE *search = NULL;
	for (search = userCachingTable[userno-1]; search->next != NULL; search=search->next) {
		;//Move pointer to LRU
	}

	METABLOCK *min_meta = NULL;
	for (; search != NULL; search=search->pre) {
		min_meta = metadataSearchByUser(search->diskBlkno, userno);
		if (min_meta == NULL)
			PrintError(-1, "[CACHE]Cache eviction error:! Meta. of victim not found!:");

		if (min_meta->prize == minPrize) {
			userCachingSpace[search->pageno] = PAGE_FLAG_FREE;
			userFreeCount[userno-1]++;
			if (search->next != NULL) {
				if (search->pre != NULL) {
					search->pre->next = search->next;
					search->next->pre = search->pre;
				}
				else
					userCachingTable[userno-1] = search->next;
			}
			else {
				if (search->pre != NULL)
					search->pre->next = NULL;
				else
					userCachingTable[userno-1] = NULL;
			}
			search->next = NULL;
			search->pre = NULL;
			return search;
		}
	}
	return NULL;
}

/*SEARCH CACHE BY USER*/
/**
 * [指定HDD Block(disksim格式)和User進行搜尋]
 * @param {unsigned long} diskBlk [指定欲搜尋的Block(disksim格式)]
 * @param {unsigned} userno [User number(1-n)]
 * @return {SSD_CACHE*} search/NULL [回傳欲搜尋的Block Pointer;NULL:未找到]
 */
SSD_CACHE *searchCACHEByUser(unsigned long diskBlk, unsigned userno) {
	SSD_CACHE *search;
	search = userCachingTable[userno-1];

	while(search != NULL) {
		if (search->diskBlkno == diskBlk) {
			return search;
		}
		else
			search = search->next;
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
	if (userFreeCount[userno-1] == 0)
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
	unsigned long pageno;
	for (pageno = userCacheStart[userno-1]; pageno < userCacheStart[userno-1]+userCacheSize[userno-1]; pageno++) {
		if (userCachingSpace[pageno] == PAGE_FLAG_FREE) {
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
	SSD_CACHE *search;
	unsigned i;
	for (i = 0; i < NUM_OF_USER; i++) {
		printf("[USER CACHE %u]<<<MRU<<<", i+1);
		
		for (search = userCachingTable[i]; search != NULL; search=search->next) {
			printf("%lu(%lu)[%lu] <-> ", search->pageno, search->diskBlkno, search->diskBlkno/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK));;
		}

		printf("NULL (%lu)\n", userCacheSize[i]-userFreeCount[i]);
	}
}

/*****Without ssd logical partition*****/

/*CACHE INITIALIZATION*/
/**
 * [設定Cache大小]
 * @return {int} 0/-1 []
 */
int initCACHE() {
	int i;
	for (i = 0; i < NUM_OF_USER; i++) {
		userCacheCount[i] = 0;
	}
	freeCount = SSD_CACHING_SPACE_BY_PAGES;
	return 0;
}

/*INSERT CACHE TABLE*/
/**
 * [插入(新增)Caching Block至Caching Table]
 * @param {unsigned long*} diskBlk [Block Number(disksim格式) in HDD]
 * @param {int} reqFlag [此次Caching是否修改Caching Block(參考PAGE_FLAG_XXXX)]
 * @param {unsigned} userno [User number(1-n)]
 * @return {int} 0/-1 [FULL(-1) or not(0)]
 */
int insertCACHE(unsigned long *diskBlk, int reqFlag, unsigned userno) {
	//新增Caching Table中第一筆資料(Table為空)
	if (cachingTable == NULL) {
		unsigned long freePage;
		if (isFullCACHE())
			return -1;//FULL
		else
			freePage = getFreeCACHE();

		cachingTable = calloc(1, sizeof(SSD_CACHE));
		cachingTable->diskBlkno = *diskBlk;
		cachingTable->dirtyFlag = reqFlag;
		cachingTable->user = userno;
		cachingTable->pre = NULL;
		cachingTable->next = NULL;
		
		cachingTable->pageno = freePage;
		cachingSpace[freePage] = reqFlag;
		userCacheCount[userno-1]++;
		freeCount--;
		//printf("cache first %lu\n", freePage);
	}
	else {//新增或更新Caching Table中某筆資料
		SSD_CACHE *search;
		search = searchCACHE(*diskBlk);

		//新增一筆資料
		if (search == NULL) {
			unsigned long freePage;
			if (isFullCACHE()){
				return -1;//FULL
			}
			else
				freePage = getFreeCACHE();

			SSD_CACHE *tmp;
			tmp = calloc(1, sizeof(SSD_CACHE));
			tmp->diskBlkno = *diskBlk;
			tmp->dirtyFlag = reqFlag;
			tmp->user = userno;

			cachingTable->pre = tmp;
			tmp->next = cachingTable;
			tmp->pre = NULL;
			cachingTable = cachingTable->pre;

			tmp->pageno = freePage;
			cachingSpace[freePage] = reqFlag;
			userCacheCount[userno-1]++;
			freeCount--;
			//printf("cache head %lu\n", freePage);
		}
		else {//更新一筆資料
			//更新此次存取的Block flag
			switch (reqFlag) {
				case PAGE_FLAG_CLEAN:
					//Dirty still dirty...
					break;
				case PAGE_FLAG_DIRTY:
					cachingSpace[search->pageno] = reqFlag;
					break;
				default:
					PrintError(reqFlag, "[CACHE]Caching Error with unknown reqFlag:");
					break;
			}

			//實現LRU順序，係為方便印出結果
			//欲cache的block不位於MRU端才需調整
			if (search->pre != NULL) {
				//此次存取的Block移至MRU端；
				if (search->next == NULL) {
					search->pre->next = NULL;
					cachingTable->pre = search;
					search->next = cachingTable;
					search->pre = NULL;
					cachingTable = cachingTable->pre;
				}
				else {
					search->pre->next = search->next;
					search->next->pre = search->pre;
					cachingTable->pre = search;
					search->next = cachingTable;
					search->pre = NULL;
					cachingTable = cachingTable->pre;
				}
			}
		}
	}
	//Update user statistics:caching space
	userst[userno-1].cachingSpace = (double)userCacheCount[userno-1]/(double)(SSD_CACHING_SPACE_BY_PAGES);
	//printCACHEByLRU();
	return 0;
}

/*CACHE EVICTION POLICY:SPECIFIC Block with min prize*/
/**
 * [根據指定的HDD Block Number進行剔除]
 * @param {double} minPrize [指定欲剔除的page是具有最小Prize的metadata Block]
 * @param {unsigned} userno [User number(1-n)]
 * @return {SSD_CACHE*} search/NULL [回傳欲剔除的Block Pointer;NULL:未找到]
 */
SSD_CACHE *evictCACHEFromLRUWithMinPrize(double minPrize, unsigned userno) {
	//printCACHEByLRU();
	SSD_CACHE *search = NULL;
	for (search = cachingTable; search->next != NULL; search=search->next) {
		;//Move pointer to LRU
	}

	METABLOCK *min_meta = NULL;
	for (; search != NULL; search=search->pre) {
		min_meta = metadataSearchByUser(search->diskBlkno, userno);
		if (min_meta == NULL)
			PrintError(-1, "[CACHE]Cache eviction error:! Meta. of victim not found!:");

		if (min_meta->prize == minPrize) {
			cachingSpace[search->pageno] = PAGE_FLAG_FREE;
			userCacheCount[search->user-1]--;
			freeCount++;
			if (search->next != NULL) {
				if (search->pre != NULL) {
					search->pre->next = search->next;
					search->next->pre = search->pre;
				}
				else
					cachingTable = search->next;
			}
			else {
				if (search->pre != NULL)
					search->pre->next = NULL;
				else
					cachingTable = NULL;
			}
			search->next = NULL;
			search->pre = NULL;
			return search;
		}
	}
	return NULL;
}

/*SEARCH CACHE*/
/**
 * [指定HDD Block(disksim格式)進行搜尋]
 * @param {unsigned long} diskBlk [指定欲搜尋的Block(disksim格式)]
 * @return {SSD_CACHE*} search/NULL [回傳欲搜尋的Block Pointer;NULL:未找到]
 */
SSD_CACHE *searchCACHE(unsigned long diskBlk) {
	SSD_CACHE *search;
	search = cachingTable;

	while(search != NULL) {
		if (search->diskBlkno == diskBlk) {
			return search;
		}
		else
			search = search->next;
	}
	return NULL;
}

/*CHECK CACHE FULL*/
/**
 * [檢查Cache是否已滿]
 * @return CACHE_FULL/CACHE_NOT_FULL [Full:1; Not full:0]
 */
int isFullCACHE() {
	if (freeCount == 0)
		return CACHE_FULL;
	else
		return CACHE_NOT_FULL;
}

/*GET FREE CACHE*/
/**
 * [取得Caching Table中一個Free Page]
 * [注意!!欲取得Free Page前必須注意已檢查是否已滿? 建議流程isFullCACHE()->getFreeCACHE()]
 * @return {unsigned long} pageno [Page Number]
 */
unsigned long getFreeCACHE() {
	unsigned long pageno;
	for (pageno = 0; pageno < SSD_CACHING_SPACE_BY_PAGES; pageno++) {
		if (cachingSpace[pageno] == PAGE_FLAG_FREE) {
			//printf("Free Page number:%lu\n", pageno);
			return pageno;
		}
	}
	PrintError(-1, "[CACHE]Get an invalid free Page number");
	//If return this means "NO FREE PAGE"!
	return pageno;
}

/*CACHING TABLE OUTPUT*/
/**
 * [印出Caching Table]
 */
void printCACHEByLRU() {
	printf("[SSD CACHE]<<<MRU<<<");
	SSD_CACHE *search;
	for (search = cachingTable; search != NULL; search=search->next) {
		printf("%lu(%lu)[%lu] <-> ", search->pageno, search->diskBlkno, search->diskBlkno/(SSD_PAGE2SECTOR*SSD_PAGES_PER_BLOCK));;
	}

	printf("NULL (%lu)\n", SSD_CACHING_SPACE_BY_PAGES-freeCount);
}
