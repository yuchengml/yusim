#include "yu_structure.h"

/*放進User Queue並轉換以Block為單位的requests*/
int insertQUE(REQ *r, USER_QUE *uq) {
	/*block_count代表此request共存取多少SSD Block*/
	int block_count;
	block_count = r->reqSize/SSD_BLOCK2SECTOR;
	int i;
	for (i = 0; i < block_count; i++) {
		if (uq->head == NULL) {
			uq->head = calloc(1, sizeof(USER_QUE_ITEM));
			uq->tail = uq->head;
			copyReq(r, &(uq->head->r));
			uq->head->r.blkno += i*SSD_BLOCK2SECTOR;
			uq->head->r.reqSize = SSD_BLOCK2SECTOR;
		}
		else {
			USER_QUE_ITEM *tmp;
			tmp = calloc(1, sizeof(USER_QUE_ITEM));
			copyReq(r, &(tmp->r));
			tmp->r.blkno += i*SSD_BLOCK2SECTOR;
			tmp->r.reqSize = SSD_BLOCK2SECTOR;
			tmp->front_req = uq->head;
			uq->head->back_req = tmp;
			uq->head = tmp;
		}
	}
	totalRequests++;
	return 0;
}

void evictQUE(USER_QUE *uq, USER_QUE_ITEM *tmp) {
	if (tmp == uq->tail) {
		if (tmp == uq->head) {
			uq->head = NULL;
			uq->tail = NULL;
		}
		else {
			uq->tail->back_req->front_req = NULL;
			uq->tail = uq->tail->back_req;
		}
	}
	else if (tmp == uq->head) {
		uq->head->front_req->back_req = NULL;
		uq->head = uq->head->front_req;
	}
	else {
		tmp->front_req->back_req = tmp->back_req;
		tmp->back_req->front_req = tmp->front_req;
	}
	
	free(tmp);
}

void printQUE(USER_QUE *uq) {
	unsigned count = 0;
	USER_QUE_ITEM *tmp = uq->tail;
	printf("[USER QUEUE]<<<");
	while (tmp != NULL) {
		count++;
		printf("%6lu <-> ", tmp->r.blkno);
		tmp = tmp->back_req;
	}
	printf("NULL (%u)\n", count);
}

/*
int main(int argc, char const *argv[]) {
	printf("This file is to define structure.\n");

	USER_QUE_ITEM *userQue;
	userQue = (USER_QUE_ITEM *)malloc(sizeof(USER_QUE_ITEM));
	REQ req = {0.5, 0, 123, 72, 0, 0};
	//userQue->request = &req;
	//userQue->next_req = NULL;
	insertQUE(userQue, req, NULL);
	evictQUE(userQue);

	userQue->next_req = (USER_QUE_ITEM *)malloc(sizeof(USER_QUE_ITEM));
	REQ req2 = {7.5, 0, 456, 36, 1, 2};
	insertQUE(userQue->next_req, req2, NULL);
	evictQUE(userQue->next_req);


	return 0;
}
*/
void copyReq(REQ *r1, REQ *r2) {
	r2->arrivalTime = r1->arrivalTime;
	r2->devno = r1->devno;
	r2->blkno = r1->blkno;
	r2->reqSize = r1->reqSize;
	r2->reqFlag = r1->reqFlag;
	r2->userno = r1->userno;
	r2->responseTime = r1->responseTime;
}

unsigned long getTotalReqs() {
	return totalRequests;
}