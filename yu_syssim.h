#ifndef YU_SYSSIM_H
#define YU_SYSSIM_H
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h> //for structure 'stat'
#include <math.h>
#include <unistd.h>  //for getpid()
#include <sys/msg.h> //for message queue
#include <pthread.h>

#include "disksim_interface.h"
#include "disksim_global.h"

#include "yu_debug.h"
#include "yu_ipc.h"
#include "yu_parameter.h"

	/*
	 * Definitions for simple system simulator that uses DiskSim as a slave.
	 *
	 * Contributed by Eran Gabber of Lucent Technologies - Bell Laboratories
	 *
	 */
	typedef	double SysTime;		/* system time in seconds.usec */


	//structure:<pre><->r<->r<-><next>
	typedef struct intq_buf_req{
		struct disksim_request req;
		struct intq_buf_req *pre;
		struct intq_buf_req *next;
	} IntqBufReq;

	/* routines for translating between the system-level simulation's simulated */
	/* time format (whatever it is) and disksim's simulated time format (a      */
	/* double, representing the number of milliseconds from the simulation's    */
	/* initialization).                                                         */

	/* In this example, system time is in seconds since initialization */
	// #define SYSSIMTIME_TO_MS(syssimtime)    (syssimtime*1e3)
	// #define MS_TO_SYSSIMTIME(curtime)       (curtime/1e3)

	/* exported by syssim_driver.c */
	void syssim_schedule_callback(disksim_interface_callback_t, SysTime t, void *);
	void ssdsim_report_completion(SysTime t, struct disksim_request *r, void *);
	void hddsim_report_completion(SysTime t, struct disksim_request *r, void *);
	void syssim_deschedule_callback(double, void *);

	void panic(const char *s);
	void exec_SSDsim(char *name, const char *parm_file, const char *output_file);
	void exec_HDDsim(char *name, const char *parm_file, const char *output_file);

	void IntqBuffering(IntqBufReq *req);
	void DeIntqBuffering();
#endif
