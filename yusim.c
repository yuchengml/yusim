#include "yusim.h"

pid_t SSDsimProc, HDDsimProc;
FILE *trace;
char *par[5];
USER_QUE userq[NUM_OF_USER];

void initDisksim() {
    pid_t procid;
    procid = fork();
    if (procid == 0) {
        SSDsimProc = getpid();
        //printf("SSDsimProc ID: %d\n", SSDsimProc);
        exec_SSDsim("SSDsim", par[1], par[2]);
        exit(0);
    }
    else if (procid < 0) {
        PrintError(-1, "SSDsim process fork() error");
        exit(1);
    }

    procid = fork();
    if (procid == 0) {
        HDDsimProc = getpid();
        //printf("HDDsimProc ID: %d\n", HDDsimProc);
        exec_HDDsim("HDDsim", par[3], par[4]);
        exit(0);
    }
    else if (procid < 0) {
        PrintError(-1, "HDDsim process fork() error");
        exit(1);
    }

    initMSQ();
}

void rmDisksim() {
    REQ *ctrl, *ctrl_rtn;
    ctrl = calloc(1, sizeof(REQ));
    ctrl_rtn = calloc(1, sizeof(REQ));
    ctrl->reqFlag = MSG_REQUEST_CONTROL_FLAG_FINISH; //非正常flag即可(ipc.c)
    
    sendFinishControl(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1);

    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_1, ctrl_rtn, MSG_TYPE_DISKSIM_1_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"SSDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime);

    sendFinishControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_2, ctrl_rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"HDDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime); 
    rmMSQ();
}

void initMSQ() {
    if(createMessageQueue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
    if(createMessageQueue(KEY_MSQ_DISKSIM_2, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
}

void rmMSQ() {
    struct msqid_ds ds;
    if(removeMessageQueue(KEY_MSQ_DISKSIM_1, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_1, "Not remove message queue:(key)");
    if(removeMessageQueue(KEY_MSQ_DISKSIM_2, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_2, "Not remove message queue:(key)");
}

void caching() {
    while (userq[0].tail != NULL) {
        prizeCaching(&userq[0].tail->r); //PRIZE.C
        evictQUE(&userq[0], userq[0].tail);
    }
}

int main(int argc, char *argv[]) {
	if (argc != 6) {
    	fprintf(stderr, "usage: %s <Trace file> <param file for SSD> <output file for SSD> <param file for HDD> <output file for HDD>\n", argv[0]);
    	exit(1);
    }
    par[0] = argv[1];
    par[1] = argv[2];
    par[2] = argv[3];
    par[3] = argv[4];
    par[4] = argv[5];

    initDisksim();
    printf("Enter to continute ...\n");
    getchar();

    double timeout = TIME_PERIOD;

    REQ *tmp, *rtn;
    tmp = calloc(1, sizeof(REQ));
    rtn = calloc(1, sizeof(REQ));

    trace = fopen(par[0], "r");
    if (!trace)
        PrintError(-1, "Input file open error");

    while(!feof(trace)) {
        fscanf(trace, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->blkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);
        //PrintREQ(tmp, "Trace");
        //getchar();

        //判斷時間間隔，用於策略的動態調整
        if (tmp->arrivalTime < timeout) {
            /*USER IDENTIFICATION*/
            insertQUE(tmp, &userq[0]);
        }
        else {
            //printf(COLOR_RB"Time to %lf\n"COLOR_N, timeout);
            //printQUE(&userq[0]);
            /*CACHING ALGORITHM*/
            caching();
            //metaTablePrint();

            /*SIMULATION*/
            sendSimulateControl(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1);
            sendSimulateControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

            PAUSE

            /*USER IDENTIFICATION*/
            insertQUE(tmp, &userq[0]);

            /*NEXT TIME PERIOD*/
            //The N-th time period = (int)(tmp->arrivalTime/TIME_PERIOD + 1);N=1~n
            timeout = (int)(tmp->arrivalTime/TIME_PERIOD + 1) * TIME_PERIOD;
        }
                
    }

    //printf(COLOR_RB"Time to %lf\n"COLOR_N, timeout);
    //printQUE(&userq[0]);
    /*THE LAST TIME PERIOD*/
    /*CACHING ALGORITHM*/
    caching();
    //metaTablePrint();

    /*SIMULATION*/
    sendSimulateControl(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1);
    sendSimulateControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

    PAUSE

    rmDisksim();

    printf(COLOR_YB"Receive requests:%lu\n"COLOR_N, getTotalReqs());
    //PRIZE.C
    pcStatistic();

	// Waiting for SSDsim and HDDsim process
    wait(NULL);
    wait(NULL);
    //OR
    //printf("Main Process waits for: %d\n", wait(NULL));
    //printf("Main Process waits for: %d\n", wait(NULL));

	exit(0);
}
