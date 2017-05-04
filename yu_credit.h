#ifndef YU_CREDIT_H
#define YU_CREDIT_H
#include <stdio.h>
#include <stdlib.h>
#include "yu_parameter.h"
#include "yu_structure.h"

	/*USER CREDIT*/
	static double userCredit[NUM_OF_USER];

	/*CREDIT INITIALIZATION*/
	double creditInit(unsigned userno);
	/*CREDIT REPLENISHMENT*/
	double creditReplenish(unsigned userno);
	/*CREDIT CHARGING*/
	double creditCharge(unsigned userno, double value);
	/*CREDIT COMPENSATION*/
	double creditCompensate(unsigned userno, double value);
	/*CREDIT-BASED SCHEDULER*/
	int creditScheduler(USER_QUE user[]);

	/*印出所有user的credit*/
	void printCredit();

#endif
