#ifndef YU_STATISTICS_H
#define YU_STATISTICS_H
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "yu_parameter.h"
	
	/*STRUCTURE DRFINITION: USER STATISTICS*/
	/**[Record statistics for each user]
	 * 是針對每一個user去記錄。有別於PCSTAT為整體系統的資訊
	 */
	typedef struct userStat {
		unsigned long totalReq;				//Request數量
		unsigned long ssdReq;				//SSD Request數量
		unsigned long totalUserReq;			//User Request數量
		unsigned long UserReqInPeriod;		//User Request in one period數量
		unsigned long UserRReq;				//User Read Request數量
		unsigned long totalSysReq;			//System Request數量
		unsigned long evictCount;			//Eviction次數
		unsigned long dirtyCount;			//Dirty Page Eviction次數
		unsigned long hitCount;				//Hit次數
		unsigned long missCount;			//Miss次數
		double resTime;						//Response time for users
		double resTimeInPeriod;				//Response time for users in one period
		double cachingSpace;				//占用SSD的比例
	} USERSTAT;

	/*USER STATISTICS*/
	USERSTAT userst[NUM_OF_USER];
	USERSTAT sysst;

	static unsigned long statPeriods = STAT_FOR_TIME_PERIODS;

	/*USER STATISTICS INITIALIZATION*/
	void initUSERSTAT();

	/*印出所有User Statistics*/
	void printUSERSTAT(double time);
	/*寫檔至 Statistics File*/
	void writeStatFile(unsigned long periods, FILE **st);
	/*印出yu_parameter.h資訊*/
	void printParameters();
	/*寫檔至 Result File*/
	void writeResultFile(FILE **result, double time);

#endif
