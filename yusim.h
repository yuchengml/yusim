#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for fork()
#include <sys/wait.h> //for wait()

#include "yu_debug.h"
#include "yu_prize.h"
#include "yu_structure.h"
#include "yu_trace.h"
#include "yu_syssim.h"
#include "yu_ipc.h"
#include "yu_credit.h"

/*Simulator*/
void initDisksim();
void rmDisksim();
void initMSQ();
void rmMSQ();

/*Caching Policy*/
void caching();
