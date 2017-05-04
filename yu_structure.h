#ifndef YU_STRUCTURE_H
#define YU_STRUCTURE_H
#include <stdlib.h>
#include <stdio.h>
#include "yu_parameter.h"
	
	//計算進入User queue的request數量，取得Request總數(應同於Trace筆數)
	static unsigned long totalRequests = 0;

	/*STRUCTURE DEFINITION:REQUEST*/
	typedef struct req {
		double arrivalTime;		//抵達時間
		unsigned devno;			//裝置編號(預設為0)
		unsigned long blkno;	//Block編號(根據Disksim格式)
		unsigned reqSize;		//Block連續數量(至少為1)
		unsigned reqFlag;		//讀:1;寫:0
		unsigned userno;		//使用者編號(1~n)
		double responseTime;	//反應時間(初始為0)
	} REQ;

	/*STRUCTURE DEFINITION:USER QUEUE*/
	typedef struct user_que_item {
		struct user_que_item *back_req;		//指向上一個(較晚進入)
		REQ r;								//REQUEST STRUCTURE
		struct user_que_item *front_req;	//指向下一個(較早進入)
	} USER_QUE_ITEM;

	/*STRUCTURE DEFINITION:USER QUEUE POINTER*/
	typedef struct user_que {
		USER_QUE_ITEM *head;		//指向佇列結構的頭
		USER_QUE_ITEM *tail;		//指向佇列結構的尾
	} USER_QUE;

	/*USER QUEUE*/
	USER_QUE userq[NUM_OF_USER];

	/*USER QUEUE INSERTION*/
	int insertQUE(REQ *r, unsigned userno);
	/*USER QUEUE EVICTION*/
	void evictQUE(unsigned userno, USER_QUE_ITEM *tmp);
	/*USER QUEUE OUTPUT*/
	void printQUE();

	/*複製Request內容, r to copy*/
	void copyReq(REQ *r, REQ *copy);
	/*取得totalRequests值*/
	unsigned long getTotalReqs();
	/*將SSD Block Number轉成Disksim Block(Sector)*/
	unsigned long ssdBlk2simSector(unsigned long ssd_blk);

#endif