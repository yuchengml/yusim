#ifndef YU_CREDIT_H
#define YU_CREDIT_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "yu_parameter.h"
#include "yu_structure.h"
#include "yu_debug.h"

	/*USER WEIGHT*/
	unsigned userWeight[NUM_OF_USER];
	unsigned totalWeight;

	/*USER CREDIT*/
	static double userCredit[NUM_OF_USER];

	/*CREDIT INITIALIZATION*/
	int creditInit();
	/*CREDIT REPLENISHMENT*/
	int creditReplenish();
	/*CREDIT CHARGING*/
	double creditCharge(unsigned userno, double value);
	/*CREDIT COMPENSATION*/
	double creditCompensate(unsigned userno, double value);
	/*CREDIT-BASED SCHEDULER*/
	int creditScheduler(USER_QUE user[], double time);

	/*印出所有user的credit*/
	void printCredit();

#endif
