#include "yu_cache.h"

int insertCACHE(unsigned long *ssd_blk, unsigned long *hdd_blk, int flag) {
	if (cachingTable == NULL) {
		int i;
		for (i = 0; i < SSD_CACHING_SPACE_BY_BLOCKS; i++) {
			if (cachingSpace[i] == BLOCK_FLAG_FREE)
				break;
		}
		if (i == SSD_CACHING_SPACE_BY_BLOCKS)
			return -1;//FULL

		cachingTable = calloc(1, sizeof(SSD_CACHE));
		cachingTable->hdd_blkno = *hdd_blk;
		cachingTable->flag = flag;
		cachingTable->pre = NULL;
		cachingTable->next = NULL;
		
		cachingTable->ssd_blkno = i;
		*ssd_blk = i;
		cachingSpace[i] = flag;
		//printf("cache first %u\n", i);
	}
	else {
		SSD_CACHE *search;
		search = searchCACHE(*hdd_blk);

		//No caching
		if (search == NULL) {
			int i;
			for (i = 0; i < SSD_CACHING_SPACE_BY_BLOCKS; i++) {
				if (cachingSpace[i] == BLOCK_FLAG_FREE)
					break;
			}
			if (i == SSD_CACHING_SPACE_BY_BLOCKS)
				return -1;//FULL

			SSD_CACHE *tmp;
			tmp = calloc(1, sizeof(SSD_CACHE));
			tmp->hdd_blkno = *hdd_blk;
			tmp->flag = flag;

			cachingTable->pre = tmp;
			tmp->next = cachingTable;
			tmp->pre = NULL;
			cachingTable = cachingTable->pre;

			

			tmp->ssd_blkno = i;
			*ssd_blk = i;
			cachingSpace[i] = flag;
			//printf("cache head %u\n", i);
		}
		else {
			//欲cache的block不位於MRU端才需調整
			if (search->pre != NULL) {
				//此次存取的block移至MRU端；
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
			//printf("move to MRU\n");
		}
	}
	return 0;
}

SSD_CACHE *evictCACHEByLRU() {
	SSD_CACHE *search = NULL;
	for (search = cachingTable; search->next != NULL; search=search->next) {
		;
	}
	//printf("evict %lu ", search->ssd_blkno);
	cachingSpace[search->ssd_blkno] = BLOCK_FLAG_FREE;
	search->pre->next = NULL;
	//printCACHEByLRU();
	//free(search);
	if (search != NULL)
		return search;
	else
		return NULL;
	
}

SSD_CACHE *evictCACHE(unsigned long hdd_blk) {
	SSD_CACHE *search = NULL;
	search = cachingTable;
	if (search->hdd_blkno == hdd_blk) {
		cachingSpace[search->ssd_blkno] = BLOCK_FLAG_FREE;
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
				search->next = tmp->next;
				if (tmp->next != NULL) 
					tmp->next->pre = search;
				//printCACHEByLRU();
				//free(tmp);
				return search;
				//return NULL;
			}
		}
	}
	return NULL;
}

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

void printCACHEByLRU() {
	printf("[SSD CACHE]<<<MRU<<<");
	SSD_CACHE *search;
	for (search = cachingTable; search != NULL; search=search->next) {
		printf("%lu(%lu) <-> ", search->ssd_blkno, search->hdd_blkno);;
	}

	int i, count=0;
	for (i = 0; i < SSD_CACHING_SPACE_BY_BLOCKS; i++) {
		if (cachingSpace[i] != BLOCK_FLAG_FREE)
			count++;
	}

	printf("NULL (%u)\n", count);
}

unsigned long ssdBlk2simSector(unsigned long ssd_blk) {
	return ssd_blk*SSD_BLOCK2SECTOR;
}
