#include "yusim.h"

pid_t SSDsimProc, HDDsimProc;
FILE *trace, *statistics;
char *par[6];
double scheduleTime = 0;
unsigned long period = 0;

/*DISKSIM INITIALIZATION*/
/**
 * [Disksim的初始化，利用兩個Process各自執行Disksim，作為SSDsim和HDDsim，
 *  接續MESSAGE QUEUE INITIALIZATION]
 */
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

/*DISKSIM SHUTDOWN*/
/**
 * [Disksim的關閉，傳送Control message 告知其Process進行Shutdown，並等待回傳結果message]
 */
void rmDisksim() {
    REQ *ctrl, *ctrl_rtn;
    ctrl = calloc(1, sizeof(REQ));
    ctrl_rtn = calloc(1, sizeof(REQ));
    ctrl->reqFlag = MSG_REQUEST_CONTROL_FLAG_FINISH; //非正常flag即可(ipc.c)
    
    sendFinishControl(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1);

    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_1, ctrl_rtn, MSG_TYPE_DISKSIM_1_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"[YUSIM]SSDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime);

    sendFinishControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_2, ctrl_rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"[YUSIM]HDDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime); 
    rmMSQ();
}

/*MESSAGE QUEUE INITIALIZATION*/
/**
 * [Message queue初始化，使用系統定義的Key值、Type和IPC function]
 */
void initMSQ() {
    if(createMessageQueue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
    if(createMessageQueue(KEY_MSQ_DISKSIM_2, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
}

/*MESSAGE QUEUE REMOVE*/
/**
 * [Message queue刪除，使用系統定義的Key值和IPC function]
 */
void rmMSQ() {
    struct msqid_ds ds;
    if(removeMessageQueue(KEY_MSQ_DISKSIM_1, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_1, "Not remove message queue:(key)");
    if(removeMessageQueue(KEY_MSQ_DISKSIM_2, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_2, "Not remove message queue:(key)");
}

/*I/O SCHEDULING*/
/**
 * [Credit-based Scheduler，並且推算系統時間，決定Request delivery(Trace->User queue)]
 * @param {double} next_timout [The arrival time of the next request waiting for queueing]
 */
void scheduling(double next_timeout) {
    double response;
    int candidate;
    
    while (1) {
        //Credit-based Scheduler
        candidate = creditScheduler(userq);
        //User Queue無任何Requests，則Stop scheduling
        if (candidate == -1) {
            //printf("[YUSIM]No any request! Stop scheduling and delivery more request into 'userq'(%lf)\n", scheduleTime);
            break;
        }

        /*CACHING ALGORITHM*/
        //回傳可得response time
        response = prizeCaching(&userq[candidate].tail->r); //PRIZE.C
        //DEBUG:
        //printCACHEByLRUandUsers();
        //metaTablePrint();
        PAUSE

        //推算執行時間，為模擬系統時間
        //若目前的時間小於下一個request抵達的時間，則將系統時間往後推至下一個request完成的時間
        if (scheduleTime < userq[candidate].tail->r.arrivalTime)
            scheduleTime = response + userq[candidate].tail->r.arrivalTime;
        else //否則，只須累加下一個request的response time即可
            scheduleTime += response;
        //printf("[YUSIM]Blkno=%lu, Response time=%lf, scheduleTime=%lf\n", userq[candidate].tail->r.diskBlkno, response, scheduleTime);
        evictQUE(candidate, userq[candidate].tail);
        
        /*USER STATISTICS*/
        userst[candidate].responseTime += response;

        /*CREDIT CHARGING*/
        //目前做法:假設一個Block Request做完後才扣Credit
        creditCharge(candidate, response);
        //printCredit();
        //creditCompensate();

        //根據TIME_PERIOD，週期性進行credit的補充與記錄實驗數據
        if (floor(scheduleTime / TIME_PERIOD) - period > 0) {
            period = floor(scheduleTime / TIME_PERIOD);
            //printf("[YUSIM]creditReplenish()[%lu](%lf)\n", period, scheduleTime);
            
            /*CREDIT REPLENISHMENT*/
            creditReplenish();
            
            /*USER STATISTICS*/
            //記錄實驗數據
            writeStatFile(scheduleTime, &statistics);
        }

        //推算系統執行時間，是否有requests要進入User queue
        if (next_timeout != -1 && next_timeout <= scheduleTime) {
            //printf("[YUSIM]Some requests coming! Stop scheduling and delivery more request into 'userq'(%lf)\n", scheduleTime);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
	if (argc != 7) {
    	fprintf(stderr, "usage: %s <Trace file> <param file for SSD> <output file for SSD> <param file for HDD> <output file for HDD> <output file for STAT>\n", argv[0]);
    	exit(1);
    }
    par[0] = argv[1];
    par[1] = argv[2];
    par[2] = argv[3];
    par[3] = argv[4];
    par[4] = argv[5];
    par[5] = argv[6];

    trace = fopen(par[0], "r");
    if (!trace)
        PrintError(-1, "[YUSIM]Input file open error");
    statistics = fopen(par[5], "w");

    printParameters();

    initDisksim();

    printf("[YUSIM]Enter to continute ...\n");
    
    getchar();

    /*GET USER WEIGHT*/
    int i;
    unsigned weight = 0;
    for(i = 0; i < NUM_OF_USER; i++) {
        fscanf(trace, "%u", &weight);
        userWeight[i] = weight;
    }
    initUserCACHE();
    creditInit();
    initUSERSTAT();

    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));

    while(!feof(trace)) {
        fscanf(trace, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);
        //PrintREQ(tmp, "Trace");

        //判斷時間間隔，模擬系統時間
        if (tmp->arrivalTime <= scheduleTime) {
            /*USER IDENTIFICATION*/
            if(insertQUE(tmp, tmp->userno-1) == -1)
                PrintError(-1, "[YUSIM]Error user or user queue! insertQUE():");
        }
        else {
            //printf(COLOR_RB"[YUSIM]Time to %lf\n"COLOR_N, scheduleTime);
            //printQUE();
            
            scheduling(tmp->arrivalTime);

            /*USER IDENTIFICATION*/
            if(insertQUE(tmp, tmp->userno-1) == -1)
                PrintError(-1, "[YUSIM]Error user or user queue! insertQUE():");
        }      
    }
    //printf(COLOR_RB"[YUSIM]Time to %lf\n"COLOR_N, scheduleTime);
    //printQUE();
    
    /*THE LAST TIME PERIOD*/
    scheduling(-1);
    //metaTablePrint();

    PAUSE

    rmDisksim();

    printf(COLOR_YB"[YUSIM]Receive requests:%lu\n"COLOR_N, getTotalReqs());
    //PRIZE.C
    pcStatistics();

    /*USER STATISTICS*/
    writeStatFile(scheduleTime, &statistics);
    printUSERSTAT(scheduleTime);

	// Waiting for SSDsim and HDDsim process
    wait(NULL);
    wait(NULL);
    //OR
    //printf("Main Process waits for: %d\n", wait(NULL));
    //printf("Main Process waits for: %d\n", wait(NULL));
    fclose(trace);
    fclose(statistics);

	exit(0);
}
