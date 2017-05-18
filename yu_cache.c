#include "yu_cache.h"
/**
 * 此檔案為維護本系統之Caching機制，根據Hybrid Storage System設定，將SSD視為HDD之Cache，
 * 因此，Caching Table須同時記錄Data(Block)位於SSD的位址(ssd_blk)和HDD的位址(hdd_blk)，以
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
	
	printf(COLOR_GB" [USER CACHE] Total User Weight:%u, Total Cache Size(Blks):%u\n"COLOR_N, totalWeight, SSD_CACHING_SPACE_BY_PAGES/SSD_PAGES_PER_BLOCK);

	for (i = 0; i < NUM_OF_USER; i++) {
		printf(COLOR_GB" [USER CACHE] User%u: Weight:%u Start(Blks):%lu, Size(Blks):%lu\n"COLOR_N, i, userWeight[i], userCacheStart[i]/SSD_PAGES_PER_BLOCK, userCacheSize[i]/SSD_PAGES_PER_BLOCK);
	}

	return 0;
}
/*INSERT CACHE TABLE BY USER*/
/**
 * [針對指定User插入(新增)Caching Block至 User Caching Table]
 * @param {unsigned long*} ssd_blk [Block Number in SSD Cache]
 * @param {unsigned long*} hdd_blk [Block Number in HDD]
 * @param {int} flag [此次Caching是否修改Caching Block(參考BLOCK_FLAG_XXXX)]
 * @param {unsigned} userno [User number(1-n)]
 * @return {int} 0/-1 [FULL(-1) or not(0)]
 */
int insertCACHEByUser(unsigned long *ssd_blk, unsigned long *hdd_blk, int flag, unsigned userno) {
	//新增Caching Table中第一筆資料(Table為空)
	if (userCachingTable[userno-1] == NULL) {
		unsigned long free_blkno;
		if (isFullCACHEByUser(userno))
			return -1;//FULL
		else
			free_blkno = getFreeCACHEByUser(userno);

		userCachingTable[userno-1] = calloc(1, sizeof(SSD_CACHE));
		userCachingTable[userno-1]->hdd_blkno = *hdd_blk;
		userCachingTable[userno-1]->dirtyFlag = flag;
		userCachingTable[userno-1]->pre = NULL;
		userCachingTable[userno-1]->next = NULL;
		
		userCachingTable[userno-1]->ssd_blkno = free_blkno;
		*ssd_blk = free_blkno;
		userCachingSpace[free_blkno] = flag;
		userFreeCount[userno-1]--;
		//printf("cache first %lu\n", free_blkno);
	}
	else {//新增或更新Caching Table中某筆資料
		SSD_CACHE *search;
		search = searchCACHE(*hdd_blk);

		//新增一筆資料
		if (search == NULL) {
			unsigned long free_blkno;
			if (isFullCACHEByUser(userno)){
				return -1;//FULL
			}
			else
				free_blkno = getFreeCACHEByUser(userno);

			SSD_CACHE *tmp;
			tmp = calloc(1, sizeof(SSD_CACHE));
			tmp->hdd_blkno = *hdd_blk;
			tmp->dirtyFlag = flag;

			userCachingTable[userno-1]->pre = tmp;
			tmp->next = userCachingTable[userno-1];
			tmp->pre = NULL;
			userCachingTable[userno-1] = userCachingTable[userno-1]->pre;

			tmp->ssd_blkno = free_blkno;
			*ssd_blk = free_blkno;
			userCachingSpace[free_blkno] = flag;
			userFreeCount[userno-1]--;
			//printf("cache head %lu\n", free_blkno);
		}
		else {//更新一筆資料
			//更新此次存取的Block Flag
			switch (flag) {
				case BLOCK_FLAG_CLEAN:
					//Dirty still dirty...
					break;
				case BLOCK_FLAG_DIRTY:
					userCachingSpace[search->ssd_blkno] = flag;
					break;
				default:
					PrintError(flag, "[USER CACHE]Caching Error with unknown flag:");
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
	return 0;
}

/*CACHE EVICTION POLICY:SPECIFIC HDD Block and User*/
/**
 * [根據指定的HDD Block Number和User Number進行剔除]
 * @param {unsigned long} hdd_blk [指定欲剔除的Block]
 * @param {unsigned} userno [User number(1-n)]
 * @return {SSD_CACHE*} search/NULL [回傳欲剔除的Block Pointer;NULL:未找到]
 */
SSD_CACHE *evictCACHEByUser(unsigned long hdd_blk, unsigned userno) {
	SSD_CACHE *search = NULL;
	search = userCachingTable[userno-1];
	//Caching Table的第一個Block為Victim
	if (search->hdd_blkno == hdd_blk) {
		userCachingSpace[search->ssd_blkno] = BLOCK_FLAG_FREE;
		userFreeCount[userno-1]++;
		search->next->pre = NULL;
		userCachingTable[userno-1] = userCachingTable[userno-1]->next;
		//free(search);
		return search;
		//return NULL;
	}
	else {
		SSD_CACHE *tmp = NULL;
		for (search = userCachingTable[userno-1]; search->next != NULL; search=search->next) {
			if (search->next->hdd_blkno == hdd_blk){
				tmp = search->next;
				userCachingSpace[search->next->ssd_blkno] = BLOCK_FLAG_FREE;
				userFreeCount[userno-1]++;
				search->next = tmp->next;
				if (tmp->next != NULL) 
					tmp->next->pre = search;
				//free(tmp);
				return tmp;
				//return NULL;
			}
		}
	}
	return NULL;
}

/*SEARCH CACHE BY USER*/
/**
 * [指定HDD Block和User進行搜尋]
 * @param {unsigned long} hdd_blk [指定欲搜尋的Block]
 * @param {unsigned} userno [User number(1-n)]
 * @return {SSD_CACHE*} search/NULL [回傳欲搜尋的Block Pointer;NULL:未找到]
 */
SSD_CACHE *searchCACHEByUser(unsigned long hdd_blk, unsigned userno) {
	SSD_CACHE *search;
	search = userCachingTable[userno-1];

	while(search != NULL) {
		if (search->hdd_blkno == hdd_blk) {
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
 * [根據User Number取得其User Caching Table中一個Free Block]
 * [注意!!欲取得Free Block前必須注意已檢查是否已滿? 建議流程isFullCACHE()->getFreeCACHE()]
 * @param {unsigned} userno [User number(1-n)]
 * @return {unsigned long}  [description]
 */
unsigned long getFreeCACHEByUser(unsigned userno) {
	unsigned long blkno;
	for (blkno = userCacheStart[userno-1]; blkno < userCacheStart[userno-1]+userCacheSize[userno-1]; blkno++) {
		if (userCachingSpace[blkno] == BLOCK_FLAG_FREE) {
			//printf("Free Block number:%lu\n", blkno);
			return blkno;
		}
	}
	PrintError(-1, "[USER CACHE]Get an invalid free block number in User Cache");
	//If return this means "NO FREE BLOCK"!
	return blkno;
}

/*CACHING TABLE OUTPUT*/
/**
 * [印出User Caching Table]
 */
void printCACHEByLRUandUsers() {
	SSD_CACHE *search;
	unsigned i;
	for (i = 0; i < NUM_OF_USER; i++) {
		printf("[USER CACHE]<<<MRU<<<");
		
		for (search = userCachingTable[i]; search != NULL; search=search->next) {
			printf("%lu(%lu) <-> ", search->ssd_blkno, search->hdd_blkno);;
		}

		printf("NULL (%lu)\n", userCacheSize[i]-userFreeCount[i]);
	}
}

/*****Without ssd logical partition*****/

/*INSERT CACHE TABLE*/
/**
 * [插入(新增)Caching Block至Caching Table]
 * @param {unsigned long*} ssd_blk [Block Number in SSD Cache]
 * @param {unsigned long*} hdd_blk [Block Number in HDD]
 * @param {int} flag [此次Caching是否修改Caching Block(參考BLOCK_FLAG_XXXX)]
 * @return {int} 0/-1 [FULL(-1) or not(0)]
 */
int insertCACHE(unsigned long *ssd_blk, unsigned long *hdd_blk, int flag) {
	//新增Caching Table中第一筆資料(Table為空)
	if (cachingTable == NULL) {
		unsigned long free_blkno;
		if (isFullCACHE())
			return -1;//FULL
		else
			free_blkno = getFreeCACHE();

		cachingTable = calloc(1, sizeof(SSD_CACHE));
		cachingTable->hdd_blkno = *hdd_blk;
		cachingTable->dirtyFlag = flag;
		cachingTable->pre = NULL;
		cachingTable->next = NULL;
		
		cachingTable->ssd_blkno = free_blkno;
		*ssd_blk = free_blkno;
		cachingSpace[free_blkno] = flag;
		freeCount--;
		//printf("cache first %lu\n", free_blkno);
	}
	else {//新增或更新Caching Table中某筆資料
		SSD_CACHE *search;
		search = searchCACHE(*hdd_blk);

		//新增一筆資料
		if (search == NULL) {
			unsigned long free_blkno;
			if (isFullCACHE()){
				return -1;//FULL
			}
			else
				free_blkno = getFreeCACHE();

			SSD_CACHE *tmp;
			tmp = calloc(1, sizeof(SSD_CACHE));
			tmp->hdd_blkno = *hdd_blk;
			tmp->dirtyFlag = flag;

			cachingTable->pre = tmp;
			tmp->next = cachingTable;
			tmp->pre = NULL;
			cachingTable = cachingTable->pre;

			tmp->ssd_blkno = free_blkno;
			*ssd_blk = free_blkno;
			cachingSpace[free_blkno] = flag;
			freeCount--;
			//printf("cache head %lu\n", free_blkno);
		}
		else {//更新一筆資料
			//更新此次存取的Block Flag
			switch (flag) {
				case BLOCK_FLAG_CLEAN:
					//Dirty still dirty...
					break;
				case BLOCK_FLAG_DIRTY:
					cachingSpace[search->ssd_blkno] = flag;
					break;
				default:
					PrintError(flag, "[CACHE]Caching Error with unknown flag:");
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
	return 0;
}

/*CACHE EVICTION POLICY:LRU*/
/**
 * [指定Cahche中LRU端的Block進行剔除]
 * @return {SSD_CACHE*} search/NULL [回傳欲剔除的Block Pointer;NULL:未找到]
 */
SSD_CACHE *evictCACHEByLRU() {
	SSD_CACHE *search = NULL;
	for (search = cachingTable; search->next != NULL; search=search->next) {
		;
	}
	//printf("evict %lu ", search->ssd_blkno);
	cachingSpace[search->ssd_blkno] = BLOCK_FLAG_FREE;
	freeCount++;
	search->pre->next = NULL;
	//printCACHEByLRU();
	//free(search);
	if (search != NULL)
		return search;
	else
		return NULL;
	
}

/*CACHE EVICTION POLICY:SPECIFIC HDD Block*/
/**
 * [根據指定的HDD Block Number進行剔除]
 * @param {unsigned long} hdd_blk [指定欲剔除的Block]
 * @return {SSD_CACHE*} search/NULL [回傳欲剔除的Block Pointer;NULL:未找到]
 */
SSD_CACHE *evictCACHE(unsigned long hdd_blk) {
	SSD_CACHE *search = NULL;
	search = cachingTable;
	//Caching Table的第一個Block為Victim
	if (search->hdd_blkno == hdd_blk) {
		cachingSpace[search->ssd_blkno] = BLOCK_FLAG_FREE;
		freeCount++;
		search->next->pre = NULL;
		cachingTable = cachingTable->next;
		//free(search);
		return search;
		//return NULL;
	}
	else {
		SSD_CACHE *tmp = NULL;
		for (search = cachingTable; search->next != NULL; search=search->next) {
			if (search->next->hdd_blkno == hdd_blk){
				tmp = search->next;
				cachingSpace[search->next->ssd_blkno] = BLOCK_FLAG_FREE;
				freeCount++;
				search->next = tmp->next;
				if (tmp->next != NULL) 
					tmp->next->pre = search;
				//free(tmp);
				return tmp;
				//return NULL;
			}
		}
	}
	return NULL;
}

/*SEARCH CACHE*/
/**
 * [指定HDD Block進行搜尋]
 * @param {unsigned long} hdd_blk [指定欲搜尋的Block]
 * @return {SSD_CACHE*} search/NULL [回傳欲搜尋的Block Pointer;NULL:未找到]
 */
SSD_CACHE *searchCACHE(unsigned long hdd_blk) {
	SSD_CACHE *search;
	search = cachingTable;

	while(search != NULL) {
		if (search->hdd_blkno == hdd_blk) {
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
 * [取得Caching Table中一個Free Block]
 * [注意!!欲取得Free Block前必須注意已檢查是否已滿? 建議流程isFullCACHE()->getFreeCACHE()]
 * @return {unsigned long}  [description]
 */
unsigned long getFreeCACHE() {
	unsigned long blkno;
	for (blkno = 0; blkno < SSD_CACHING_SPACE_BY_PAGES; blkno++) {
		if (cachingSpace[blkno] == BLOCK_FLAG_FREE) {
			//printf("Free Block number:%lu\n", blkno);
			return blkno;
		}
	}
	PrintError(-1, "[CACHE]Get an invalid free block number");
	//If return this means "NO FREE BLOCK"!
	return blkno;
}

/*CACHING TABLE OUTPUT*/
/**
 * [印出Caching Table]
 */
void printCACHEByLRU() {
	printf("[SSD CACHE]<<<MRU<<<");
	SSD_CACHE *search;
	for (search = cachingTable; search != NULL; search=search->next) {
		printf("%lu(%lu) <-> ", search->ssd_blkno, search->hdd_blkno);;
	}

	printf("NULL (%lu)\n", SSD_CACHING_SPACE_BY_PAGES-freeCount);
}
