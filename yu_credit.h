#ifndef YU_CREDIT_H
#define YU_CREDIT_H
#include <stdio.h>
#include <stdlib.h>
#include "yu_parameter.h"
#include "yu_structure.h"

	typedef double credit;

	credit creditInit(USER_QUE *user);
	credit creditReplenish(USER_QUE *user);
	credit creditCharge(USER_QUE *user, credit value);
	credit creditCompensate(USER_QUE *user, credit value);
	int creditScheduler(USER_QUE user[]);

#endif
