#ifndef YU_STRUCTURE_H
#define YU_STRUCTURE_H
#include <stdlib.h>
#include <stdio.h>
#include "yu_parameter.h"

	static unsigned long totalRequests = 0;
	typedef struct req {
		double arrivalTime;
		unsigned devno;
		unsigned long blkno;
		unsigned reqSize;
		unsigned reqFlag;
		unsigned userno;
		double responseTime;
	} REQ;

	typedef struct user_que_item {
		struct user_que_item *back_req;
		REQ r;
		struct user_que_item *front_req;
	} USER_QUE_ITEM;

	typedef struct user_que {
		USER_QUE_ITEM *head;
		USER_QUE_ITEM *tail;
	} USER_QUE;


	int insertQUE(REQ *r, USER_QUE *uq);
	void evictQUE(USER_QUE *uq, USER_QUE_ITEM *tmp);
	void printQUE(USER_QUE *uq);
	void copyReq(REQ *r1, REQ *r2);
	unsigned long getTotalReqs();

#endif