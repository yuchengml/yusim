#include "yu_syssim.h"

static SysTime now = 0;     /* current time *//*本程式將累積此值作為系統整體的response time*/
static SysTime next_event = -1; /* next event */
static int completed = 0;   /* requests completed */ // >0:reqs待處理 0:reqs完成(初始)
//static Stat st;
//Modify response time
static SysTime /*lastnow = 0, offset = 0, */sysResponse = 0;

static IntqBufReq *IBRhead = NULL;
static int IntqBufSize = 0;

void panic(const char *s) {
	perror(s);
	exit(1);
}

void IntqBuffering(IntqBufReq *req) {
    if (IBRhead == NULL) {
        IBRhead = req;
        IntqBufSize = 1;
    }
    else {
        req->next = IBRhead;
        IBRhead->pre = req;
        IBRhead = req;
        IntqBufSize++;
    }
}

void DeIntqBuffering() {
    IntqBufReq *tmp;
    while(IBRhead != NULL) {
        tmp = IBRhead;
        IBRhead = IBRhead->next;
        free(tmp);
    }
    IntqBufSize = 0;
}

/*
 * Schedule next callback at time t.
 * Note that there is only *one* outstanding callback at any given time.
 * The callback is for the earliest event.
 */
void syssim_schedule_callback(disksim_interface_callback_t fn, SysTime t, void *ctx) {
	next_event = t;
    now = next_event;
    //printf("Now = %lf , next_event = %lf\n", now, next_event);
}

/* de-scehdule a callback. */
void syssim_deschedule_callback(double t, void *ctx) {
	next_event = -1;
}

/*針對一個Request做完後回饋*/
/*若為一連串的request, 則回傳的r將視為最後一個*/
void ssdsim_report_completion(SysTime t, struct disksim_request *r, void *ctx) {
	completed--;
    now = t;
    sysResponse += (t-r->start);
    //printf("[SSDSIM]sysResponse1 = %lf, start = %lf, *responseTime = %lf, blkno = %lu\n", t, r->start, t-r->start, r->blkno);
    /*
    if (lastnow <= offset) {
        printf("[SSDSIM]sysResponse1 = %lf, start = %lf, *responseTime = %lf, blkno = %lu\n", t, r->start, t-r->start, r->blkno);
        sysResponse = t;
    }
    else {
        printf("[SSDSIM]sysResponse2 = %lf, start = %lf, *responseTime = %lf, blkno = %lu\n", lastnow+t-offset, r->start, lastnow+t-offset-r->start, r->blkno);
        sysResponse = lastnow+t-offset;
    }
	*/
    //printf("Now = %lf\n", now);
}

void hddsim_report_completion(SysTime t, struct disksim_request *r, void *ctx) {
    completed--;
    now = t;
    sysResponse += (t-r->start);
    //printf("[HDDSIM]sysResponse1 = %lf, start = %lf, *responseTime = %lf, blkno = %lu\n", t, r->start, t-r->start, r->blkno);
    /*
    if (lastnow <= offset) {
        printf("[HDDSIM]sysResponse1 = %lf, start = %lf, *responseTime = %lf, blkno = %lu\n", t, r->start, t-r->start, r->blkno);
        sysResponse = t;
    }
    else {
        printf("[HDDSIM]sysResponse2 = %lf, start = %lf, *responseTime = %lf, blkno = %lu\n", lastnow+t-offset, r->start, lastnow+t-offset-r->start, r->blkno);
        sysResponse = lastnow+t-offset;
    }
    */
    //printf("Now = %lf\n", now);
}

void exec_SSDsim(char *name, const char *parm_file, const char *output_file) {
    printf("[SSDSIM INFO]Disksim name:%s (%d)\n[SSDSIM INFO]Parameter File:%s\n[SSDSIM INFO]Output File:%s\n", name, getpid(), parm_file, output_file);

    struct disksim_interface *disksim;
    struct stat buf;
    IntqBufReq *pendReq;

    if (stat(parm_file, &buf) < 0)
        panic(parm_file);

    disksim = disksim_interface_initialize(parm_file, 
                                         output_file,
                                         ssdsim_report_completion,
                                         syssim_schedule_callback,
                                         syssim_deschedule_callback,
                                         0,
                                         0,
                                         0);
    PrintSomething(">>>>>[SSDSIM] Launch!");

    REQ *rp = NULL;
	rp = calloc(1, sizeof(REQ));
    for (;;) {
		//
    	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_1, rp, MSG_TYPE_DISKSIM_1) == -1)
    	    PrintError(-1, "[SSDSIM]A request not received from MSQ in recvRequestByMSQ():");
    	
    	//PrintREQ(rp ,"From user to SSDsim");
		
		if (rp->reqFlag == MSG_REQUEST_CONTROL_FLAG_FINISH) {
			REQ *ctrl;
    		ctrl = calloc(1, sizeof(REQ));
    		ctrl->responseTime = sysResponse;
    		if(sendRequestByMSQ(KEY_MSQ_DISKSIM_1, ctrl, MSG_TYPE_DISKSIM_1_SERVED) == -1)
    		    PrintError(KEY_MSQ_DISKSIM_1, "A control request not sent to MSQ in sendRequestByMSQ() return:");
    		
    		disksim_interface_shutdown(disksim, now);
    		PrintSomething("<<<<<[SSDSIM] Shutdown!");
    		exit(0);
		}
		else if (rp->reqFlag == MSG_REQUEST_CONTROL_FLAG_SIMULATE) {
            /* Process events until this I/O is completed */
            if (completed != 0) {
                while(next_event >= 0) {
                    now = next_event;
                    next_event = -1;
                    disksim_interface_internal_event(disksim, now, 0);
                }

                if (completed != 0) {
                    PrintError(completed, "[SSDSIM]internal error. Some events not completed:");
                    //fprintf(stderr, "[SSDSIM]internal error. Last event not completed \n");
                    exit(1);
                }
            }
            //Delete pending requests by de-buffering
            DeIntqBuffering();
            //completed = -1;
            //Modify response time
            //printf("LASTNOW:%lf  OFFSET:%lf\n", lastnow, offset);
            /*
            if (lastnow <= offset) {
                lastnow = now;
            }
            else {
                lastnow = lastnow + (now - offset);
            }
            offset = 0;
            */
    	}
    	else {
            pendReq = calloc(1, sizeof(IntqBufReq));
    		if (rp->reqFlag == 1)
    			pendReq->req.flags = DISKSIM_READ;
    		else if (rp->reqFlag == 0)
    			pendReq->req.flags = DISKSIM_WRITE;

            //Modify response time
            /*
            if (lastnow != 0 && offset == 0)
                offset = rp->arrivalTime;
            */
			pendReq->req.start = rp->arrivalTime;
			pendReq->req.devno = rp->devno;
			/*
			 *blkno: 單位為512Bytes。
			 *以HDD而言，大小任意；
			 *以SSD而言，4K對齊，則blkno須為8的整數倍
			 *blkno=>第?個sector (512Bytes)
			 *bytecount=>一共存取?個sector
			*/
			pendReq->req.blkno = rp->blkno;//*(SSD_BLOCK2SECTOR);
			pendReq->req.bytecount = rp->reqSize*DISKSIM_SECTOR;
			/*
			printf("pendReq->req.start=%lf\n", pendReq->req.start);
			printf("pendReq->req.flags=%u\n", pendReq->req.flags);
			printf("pendReq->req.devno=%u\n", pendReq->req.devno);
			printf("pendReq->req.blkno=%lu\n", pendReq->req.blkno);
			printf("pendReq->req.bytecount=%u\n", pendReq->req.bytecount);
			*/
			//
            //Record pending requests by buffering
            IntqBuffering(pendReq);
			/*
			if (now < pendReq->req.start)
				now = pendReq->req.start;
			*/
		    completed++;
		    //為何傳入now作為系統時間,而非request的arrival time 是因為取得request發出-完成之response time
		    //disksim_interface_request_arrive(disksim, now, &pendReq->req);
            disksim_interface_request_arrive(disksim, pendReq->req.start, &pendReq->req);
    	}
    }
}

void exec_HDDsim(char *name, const char *parm_file, const char *output_file) {
    printf("[HDDSIM INFO]Disksim name:%s (%d)\n[HDDSIM INFO]Parameter File:%s\n[HDDSIM INFO]Output File:%s\n", name, getpid(), parm_file, output_file);

    struct disksim_interface *disksim;
    struct stat buf;
    IntqBufReq *pendReq;

    if (stat(parm_file, &buf) < 0)
        panic(parm_file);

    disksim = disksim_interface_initialize(parm_file, 
                                         output_file,
                                         hddsim_report_completion,
                                         syssim_schedule_callback,
                                         syssim_deschedule_callback,
                                         0,
                                         0,
                                         0);
    PrintSomething(">>>>>[HDDSIM] Launch!");

    REQ *rp = NULL;
	rp = calloc(1, sizeof(REQ));
    for (;;) {
		if(recvRequestByMSQ(KEY_MSQ_DISKSIM_2, rp, MSG_TYPE_DISKSIM_2) == -1)
    	    PrintError(-1, "[HDDSIM]A request not received from MSQ in recvRequestByMSQ():");
    	
        //PrintREQ(rp ,"From user to HDDsim");
		
		if (rp->reqFlag == MSG_REQUEST_CONTROL_FLAG_FINISH) {
			REQ *ctrl;
    		ctrl = calloc(1, sizeof(REQ));
    		ctrl->responseTime = sysResponse;
    		if(sendRequestByMSQ(KEY_MSQ_DISKSIM_2, ctrl, MSG_TYPE_DISKSIM_2_SERVED) == -1)
    		    PrintError(KEY_MSQ_DISKSIM_2, "A control request not sent to MSQ in sendRequestByMSQ() return:");
    		
    		disksim_interface_shutdown(disksim, now);
    		PrintSomething("<<<<<[HDDSIM] Shutdown!");
    		exit(0);
		}
		else if (rp->reqFlag == MSG_REQUEST_CONTROL_FLAG_SIMULATE) {
    		/* Process events until this I/O is completed */
            if (completed != 0) {
                while(next_event >= 0) {
                    now = next_event;
                    next_event = -1;
                    disksim_interface_internal_event(disksim, now, 0);
                }

                if (completed != 0) {
                    PrintError(completed, "[HDDSIM]internal error. Some events not completed:");
                    //fprintf(stderr, "[HDDSIM]internal error. Last event not completed \n");
                    exit(1);
                }
            }
            //Delete pending requests by de-buffering
            DeIntqBuffering();
            //completed = -1;
            //Modify response time
            //printf("LASTNOW:%lf  OFFSET:%lf\n", lastnow, offset);
            /*
            if (lastnow <= offset) {
                lastnow = now;
            }
            else {
                lastnow = lastnow + (now - offset);
            }
            offset = 0;
            */
    	}
    	else {
            pendReq = calloc(1, sizeof(IntqBufReq));
    		if (rp->reqFlag == 1)
    			pendReq->req.flags = DISKSIM_READ;
    		else if (rp->reqFlag == 0)
    			pendReq->req.flags = DISKSIM_WRITE;

            //Modify response time
            /*
            if (lastnow != 0 && offset == 0)
                offset = rp->arrivalTime;
            */

			pendReq->req.start = rp->arrivalTime;
			pendReq->req.devno = rp->devno;
			/*
			 *blkno: 單位為512Bytes。
			 *以HDD而言，大小任意；
			 *以SSD而言，4K對齊，則blkno須為8的整數倍
			 *blkno=>第?個sector (512Bytes)
			 *bytecount=>一共存取?個sector
			*/
			pendReq->req.blkno = rp->blkno;//*HDD_BLOCK2SECTOR;
			pendReq->req.bytecount = rp->reqSize*DISKSIM_SECTOR;
			/*
			printf("pendReq->req.start=%lf\n", pendReq->req.start);
			printf("pendReq->req.flags=%u\n", pendReq->req.flags);
			printf("pendReq->req.devno=%u\n", pendReq->req.devno);
			printf("pendReq->req.blkno=%lu\n", pendReq->req.blkno);
			printf("pendReq->req.bytecount=%u\n", pendReq->req.bytecount);
			*/
			//
            //Record pending requests by buffering
            IntqBuffering(pendReq);
			/*
			if (now < pendReq->req.start)
				now = pendReq->req.start;
			*/
		    completed++;
		    //為何傳入now作為系統時間,而非request的arrival time 是因為取得request發出-完成之response time
		    //disksim_interface_request_arrive(disksim, now, &pendReq->req);
            disksim_interface_request_arrive(disksim, pendReq->req.start, &pendReq->req);
            
    	}
    }
}

