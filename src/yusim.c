#include "yusim.h"

pid_t SSDsimProc, HDDsimProc;           //Sub-process id: SSD and HDD simulator
FILE *trace, *statistics, *result;      //Trace file; Stat file; Result file
char *par[6];                           //Arguments in yusim execution
double scheduleTime = 0;                //Simulation time
unsigned long period = 0;               //The counter of time period

/*DISKSIM INITIALIZATION*/
/**
 * [Disksim的初始化，利用兩個Process各自執行Disksim，作為SSDsim和HDDsim，
 *  接續MESSAGE QUEUE INITIALIZATION]
 */
void initDisksim() {
    pid_t procid;
    //Fork process to execute SSD simulator
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

    //Fork process to execute HDD simulator
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

    //After the initialization of simulators, initialize message queue
    initMSQ();
}

/*DISKSIM SHUTDOWN*/
/**
 * [Disksim的關閉，傳送Control message告知其Process進行Shutdown，並等待回傳結果message]
 */
void rmDisksim() {
    REQ *ctrl, *ctrl_rtn;
    ctrl = calloc(1, sizeof(REQ));
    ctrl_rtn = calloc(1, sizeof(REQ));      //Receive message after control message
    ctrl->reqFlag = MSG_REQUEST_CONTROL_FLAG_FINISH; //Assign a finish flag (ipc)
    
    //Send a control message to finish SSD simulator
    sendFinishControl(KEY_MSQ_DISKSIM_1, MSG_TYPE_DISKSIM_1);

    //Receive the last message from SSD simulator
    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_1, ctrl_rtn, MSG_TYPE_DISKSIM_1_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"[YUSIM]SSDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime);
    fprintf(result, "[YUSIM]SSDsim response time = %lf\n", ctrl_rtn->responseTime);

    //Send a control message to finish HDD simulator
    sendFinishControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

    //Receive the last message from HDD simulator
    if(recvRequestByMSQ(KEY_MSQ_DISKSIM_2, ctrl_rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
            PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
    printf(COLOR_YB"[YUSIM]HDDsim response time = %lf\n"COLOR_N, ctrl_rtn->responseTime);
    fprintf(result, "[YUSIM]HDDsim response time = %lf\n", ctrl_rtn->responseTime);

    //After that, remove message queues
    rmMSQ();
}

/*MESSAGE QUEUE INITIALIZATION*/
/**
 * [Message queue初始化，使用系統定義的Key值、Type和IPC function]
 */
void initMSQ() {
    //Create message queue for SSD simulator
    if(createMessageQueue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
    //Create message queue for HDD simulator
    if(createMessageQueue(KEY_MSQ_DISKSIM_2, IPC_CREAT) == -1)
        PrintError(-1, " MSQ create error in createMessageQueue():");
}

/*MESSAGE QUEUE REMOVE*/
/**
 * [Message queue刪除，使用系統定義的Key值和IPC function]
 */
void rmMSQ() {
    struct msqid_ds ds;
    //Remove message queue for SSD simulator
    if(removeMessageQueue(KEY_MSQ_DISKSIM_1, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_1, "Not remove message queue:(key)");
    //Remove message queue for HDD simulator
    if(removeMessageQueue(KEY_MSQ_DISKSIM_2, &ds) == -1)
        PrintError(KEY_MSQ_DISKSIM_2, "Not remove message queue:(key)");
}

/*I/O SCHEDULING*/
/**
 * [Credit-based Scheduler，並且推算系統時間，決定Request delivery(Trace->User queue)]
 */
void scheduling() {
    int i;      //For loop
    unsigned long shiftingPeriods;      //Counting time periods in idle time
    double response, service;           //Record response time and service time
    int candidate;                      //Record the user number determined by scheduler
    
    while (1) {
        //Credit-based Scheduler
        //Determine which user request will be served
        candidate = creditScheduler(userq, scheduleTime);

        #ifdef YUSIM_CREDIT_BASED_SCHEDULER_WORK_CONSERVING
            //No request in user queues, stop scheduling
            if (candidate == -1) {
                break;
            }
        #else
            //No request in user queues, stop scheduling
            if (candidate == -1) {
                break;
            }
            else if (candidate == -2) {//No suitable request in this period
                //Push time periods until scheduler finds a suitable request(user with credit)
                while(candidate == -2) {
                    //Push one time period
                    period++;
                    scheduleTime = period * TIME_PERIOD;

                    //Stat file
                    writeStatFile(period, &statistics);
                    
                    //Progress output
                    printProgress(sysst.totalUserReq, getTotalReqs(), getMetaCnt(), getCacheCnt());

                    //Credit replenishment
                    //printf("[YUSIM]creditReplenish()[%lu](%lf)\n", period, scheduleTime);
                    creditReplenish();

                    //Credit-based Scheduler
                    //Determine which user request will be served
                    candidate = creditScheduler(userq, scheduleTime);
                }
            }
        #endif
        
        //Caching algorithm:Prize Caching
        //Send the pending request and return the service time
        service = prizeCaching(&userq[candidate].tail->r, scheduleTime); //(prize)
        //DEBUG:
        //printCACHEByLRUandUsers();
        //metaTablePrint();
        PAUSE

        //Calculate simulation time
        if (scheduleTime < userq[candidate].tail->r.arrivalTime)
            scheduleTime = userq[candidate].tail->r.arrivalTime + service;
        else
            scheduleTime += service;

        //Calculate response time
        response = scheduleTime - userq[candidate].tail->r.arrivalTime;
        
        //DEBUG:
        //printCredit();
        //printf("\n[%d]arrival = %lf, scheduleTime = %lf, service = %lf, response = %lf\n", candidate, userq[candidate].tail->r.arrivalTime, scheduleTime, service, response);
        //getchar();

        //After service, remove the served request from user queue
        evictQUE(candidate, userq[candidate].tail);
        
        //User statistics
        userst[candidate].resTime += response;
        userst[candidate].resTimeInPeriod += response;
        sysst.resTime += response;
        sysst.resTimeInPeriod += response;

        //Credit charging
        //Current practice:Charging after service
        creditCharge(candidate, service);
        //DEBUG:
        //printCredit();

        //Un-used
        //creditCompensate();

        //According to TIME_PERIOD, replenish credit and 
        shiftingPeriods = floor(scheduleTime / TIME_PERIOD) - period;
        for (i = 1; i <= shiftingPeriods; i++) {
            //New time period
            period++;

            //Stat file
            writeStatFile(period, &statistics);
            
            //Progress output
            printProgress(sysst.totalUserReq, getTotalReqs(), getMetaCnt(), getCacheCnt());

            /*CREDIT REPLENISHMENT*/
            //printf("[YUSIM]creditReplenish()[%lu](%lf)\n", period, scheduleTime);
            creditReplenish();
        }
    }
}

int main(int argc, char *argv[]) {
    //Check arguments
	if (argc != 8) {
    	fprintf(stderr, "usage: %s <Trace file> <param file for SSD> <output file for SSD> <param file for HDD> <output file for HDD> <output file for STAT> <output file for result>\n", argv[0]);
    	exit(1);
    }

    //Record arguments by variables
    par[0] = argv[1];
    par[1] = argv[2];
    par[2] = argv[3];
    par[3] = argv[4];
    par[4] = argv[5];
    par[5] = argv[6];
    par[6] = argv[7];

    //Open trace file
    trace = fopen(par[0], "r");
    if (!trace)
        PrintError(-1, "[YUSIM]Input file open error");
    //Open stat file
    statistics = fopen(par[5], "w");
    //Open result file
    result = fopen(par[6], "w");

    //Parameter(arguments) output
    printParameters();

    //Initialize Disksim(SSD and HDD simulators)
    initDisksim();

    //Checkpoint
    printf("[YUSIM]Enter to continute ...\n");
    //getchar();

    //Calculate user weights
    int i;
    unsigned weight = 0;
    for(i = 0; i < NUM_OF_USER; i++) {
        fscanf(trace, "%u", &weight);
        userWeight[i] = weight;
    }

    //Initialize user cache by weights
    initUserCACHE();
    //Initialize credit by weights
    creditInit();
    //Initialize metadata table in pc
    initmetaTable();
    //Initialize user statistics
    initUSERSTAT();

    //From trace file, insert user requests into user queues
    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));
    printf("[YUSIM]Trace loading...............");
    while(!feof(trace)) {
        fscanf(trace, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);
        //DEBUG:
        //PrintREQ(tmp, "Trace");

        //User identification and insert queue
        if(insertQUE(tmp, tmp->userno-1) == -1)
            PrintError(-1, "[YUSIM]Error user or user queue! insertQUE():");  
    }
    printf("[Finish]\n");
    
    //DEBUG:
    //printQUE();
    
    //printf("start scheduling...\n");
    scheduling();
    //printf("finish scheduling...\n");

    //Progress output
    printProgress(sysst.totalUserReq, getTotalReqs(), getMetaCnt(), getCacheCnt());
    printf ("\n");  //In need of printProgress()
    
    //DEBUG:
    //metaTablePrint();
    PAUSE

    //Remove Disksim(SSD and HDD simulators)
    rmDisksim();

    //Total requests output
    printf(COLOR_YB"[YUSIM]Receive requests:%lu\n"COLOR_N, getTotalReqs());
    
    //Statistics output in Prize caching (prize)
    pcStatistics();

    //User statistics output
    printUSERSTAT(scheduleTime);

    //Result file
    fprintf(result, "[YUSIM]Receive requests:%lu\n", getTotalReqs());
    CACHEWriteResultFile(&result);
    pcWriteResultFile(&result);
    writeResultFile(&result, scheduleTime);

	// Waiting for SSDsim and HDDsim process
    wait(NULL);
    wait(NULL);
    //OR
    //printf("Main Process waits for: %d\n", wait(NULL));
    //printf("Main Process waits for: %d\n", wait(NULL));

    //Release files items
    fclose(trace);
    fclose(statistics);
    fclose(result);

	exit(0);
}
