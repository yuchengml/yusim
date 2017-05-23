#include "yu_structure.h"

/*USER QUEUE INSERTION*/
/**
 * [放進User Queue並轉換以Page為單位的requests]
 * @param {REQ*} r [Yusim-defined request pointer]
 * @param {unsigned} userno [User number]
 * @return {int} 0/-1 [Error flag(True or False)]
 */
int insertQUE(REQ *r, unsigned userno) {
	if (userno > NUM_OF_USER-1) {
		return -1;
	}
	/*page_count代表此request共存取多少SSD Block*/
	int page_count;
	page_count = r->reqSize/SSD_PAGE2SECTOR;
	int i;
	for (i = 0; i < page_count; i++) {
		if (userq[userno].head == NULL) {
			userq[userno].head = calloc(1, sizeof(USER_QUE_ITEM));
			userq[userno].tail = userq[userno].head;
			copyReq(r, &(userq[userno].head->r));
			userq[userno].head->r.diskBlkno += i*SSD_PAGE2SECTOR;
			userq[userno].head->r.reqSize = SSD_PAGE2SECTOR;
		}
		else {
			USER_QUE_ITEM *tmp;
			tmp = calloc(1, sizeof(USER_QUE_ITEM));
			copyReq(r, &(tmp->r));
			tmp->r.diskBlkno += i*SSD_PAGE2SECTOR;
			tmp->r.reqSize = SSD_PAGE2SECTOR;
			tmp->front_req = userq[userno].head;
			userq[userno].head->back_req = tmp;
			userq[userno].head = tmp;
		}
	}
	totalRequests++;
	return 0;
}

/*USER QUEUE EVICTION*/
/**
 * [根據欲剔除的user queue item進行剔除，並釋放記憶體]
 * @param {unsigned} userno [User number]
 * @param {USER_QUE_ITEM*} tmp [User queue item pointer]
 */
void evictQUE(unsigned userno, USER_QUE_ITEM *tmp) {
	if (tmp == userq[userno].tail) {
		if (tmp == userq[userno].head) {
			userq[userno].head = NULL;
			userq[userno].tail = NULL;
		}
		else {
			userq[userno].tail->back_req->front_req = NULL;
			userq[userno].tail = userq[userno].tail->back_req;
		}
	}
	else if (tmp == userq[userno].head) {
		userq[userno].head->front_req->back_req = NULL;
		userq[userno].head = userq[userno].head->front_req;
	}
	else {
		tmp->front_req->back_req = tmp->back_req;
		tmp->back_req->front_req = tmp->front_req;
	}
	
	free(tmp);
}

/*USER QUEUE OUTPUT*/
/**
 * [印出所有User queue的內容]
 */
void printQUE() {
	int i;
	unsigned count;
	USER_QUE_ITEM *tmp;
	for (i = 0; i < NUM_OF_USER; i++) {
		count = 0;
		tmp = userq[i].tail;
		printf("[USER QUEUE][%d]<<<", i);
		while (tmp != NULL) {
			count++;
			printf("%6lu <-> ", tmp->r.diskBlkno);
			tmp = tmp->back_req;
		}
		printf("NULL (%u)\n", count);
	}
}

/**
 * [複製Request內容, r to copy]
 * @param {REQ*} r [Yusim-defined request pointer]
 * @param {REQ*} copy [Yusim-defined request pointer]
 */
void copyReq(REQ *r, REQ *copy) {
	copy->arrivalTime = r->arrivalTime;
	copy->devno = r->devno;
	copy->diskBlkno = r->diskBlkno;
	copy->reqSize = r->reqSize;
	copy->reqFlag = r->reqFlag;
	copy->userno = r->userno;
	copy->responseTime = r->responseTime;
}

/**
 * [根據進入User queue的request數量，取得Request總數(應同於Trace筆數)]
 * @return {unsigned long} totalRequests [The num of requests]
 */
unsigned long getTotalReqs() {
	return totalRequests;
}

/**
 * [將SSD Page Number轉成Disksim Block(Sector)]
 * @param {unsigned long} ssdPageno [SSD Page Number]
 * @return {unsigned long} - [Block(Sector) Number for SSDsim(Disksim)]
 */
unsigned long ssdPage2simSector(unsigned long ssdPageno) {
	return ssdPageno*SSD_PAGE2SECTOR;
}