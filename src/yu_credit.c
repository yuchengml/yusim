#include "yu_credit.h"

/*CREDIT INITIALIZATION*/
/**[針對所有Users初始化Credit]
 * @return {int} 0/-1 [success/fail]
 */
int creditInit() {
    //Check the sum of user weights 
    if (totalWeight == 0) {
        PrintError(totalWeight, "[CREDIT]You need to call initUserCACHE() first due to error totalWeight = ");
        return -1;
    }

    //Distribute user credit to users by the individual user weight
    int i;
    for(i = 0; i < NUM_OF_USER; i++) {
        userCredit[i] = INIT_CREDIT * ((double)userWeight[i]/totalWeight);
    }

    //User credit output
    printCredit();
    printf(COLOR_GB" [CREDIT]creditInit() finish!\n"COLOR_N);
    return 0;
}

/*CREDIT REPLENISHMENT*/
/**
 * [針對所有Users補充Credit]
 * @return {int} 0/-1 [success/fail]
 */
int creditReplenish(unsigned userno) {
    //Check the sum of user weights 
    if (totalWeight == 0) {
        PrintError(totalWeight, "[CREDIT]Error totalWeight = ");
        return -1;
    }
    
    //Distribute user credit to users by the individual user weight
    //Replenishment policy: User credit is negative or positive
    int i;
    for(i = 0; i < NUM_OF_USER; i++) {
        if (userCredit[i] < 0) //User credit is negative
            userCredit[i] += INIT_CREDIT * ((double)userWeight[i]/totalWeight);
        else //User credit is positive
            userCredit[i] = INIT_CREDIT * ((double)userWeight[i]/totalWeight);
    }
    
    //DEBUG:
    //printCredit();
    //printf(COLOR_GB" [CREDIT]creditReplenish() finish!\n"COLOR_N);
    return 0;
}

/*CREDIT CHARGING*/
/**
 * [消耗Credit，根據User number]
 * @param {unsigned} userno [User number(0-n)]
 * @param {double} value [補充量]
 * @return {double} userCredit [Modified user credit]
 */
double creditCharge(unsigned userno, double value) {
    //Charge credit
    userCredit[userno] -= value;
    return userCredit[userno];
}

/*CREDIT COMPENSATION*/
/**
 * [多扣少補Credit，根據User number]
 * @param {unsigned} userno [User number(0-n)]
 * @param {double} value [補充量]
 * @return {double} userCredit [Modified user credit]
 */
double creditCompensate(unsigned userno, double value) {
    //Compensate credit
    userCredit[userno] += value;
    return userCredit[userno];
}

/*CREDIT-BASED SCHEDULER*/
/**
 * [根據Credit策略，判斷哪個user允許服務其request]
 * @param {USER_QUE} user[] [ All of user queues]
 * @return {int} candidate [下一個候選user，作為下一個request的依據]
 */
int creditScheduler(USER_QUE user[], double time) {
    int i;              //For loop
    int candidate=-1;   //Candidate user 
    double max=0;       //The maximal credit

    #ifdef YUSIM_CREDIT_BASED_SCHEDULER
        //Find the user with the maximal credit and pending requests
        for (i = 0; i < NUM_OF_USER; i++) {
            if (user[i].tail != NULL && user[i].tail->r.arrivalTime <= time) {
                if (userCredit[i] > max) {
                    max = userCredit[i];
                    candidate = i;
                }
                else if (userCredit[i] == max && max != 0){
                    if(user[i].tail->r.arrivalTime < user[candidate].tail->r.arrivalTime) {
                        max = userCredit[i];
                        candidate = i;
                    }
                }
            }
        }

        //Get candidate
        if (candidate != -1) {
            return candidate;
        }
    #endif

    #ifdef YUSIM_CREDIT_BASED_SCHEDULER_WORK_CONSERVING
        //Find the user with a earliest pending request
        for (i = 0; i < NUM_OF_USER; i++) {
            if (user[i].tail != NULL && user[i].tail->r.arrivalTime <= time) {
                if (candidate == -1) {
                    candidate = i;
                }
                else if(user[i].tail->r.arrivalTime < user[candidate].tail->r.arrivalTime) {
                    candidate = i;
                }
            }
        }

        //Get candidate
        if (candidate != -1) {
            return candidate;
        }

        //Find the user with a earliest request which is not arrived
        for (i = 0; i < NUM_OF_USER; i++) {
            if (user[i].tail != NULL) {
                if (candidate == -1) {
                    candidate = i;
                }
                else if(user[i].tail->r.arrivalTime < user[candidate].tail->r.arrivalTime) {
                    candidate = i;
                }
            }
        }

        //Get candidate or finish(return -1)
        return candidate;
    #else
        //Find the user with credit and a earliest request which is not arrived in this period
        //Reason: Credit charging triggered across periods. User may regain priority in the next period!
        for (i = 0; i < NUM_OF_USER; i++) {
            if (user[i].tail != NULL && userCredit[i] > 0) {
                //Check the request arrived in the same period
                if (floor(time / TIME_PERIOD) == floor(user[i].tail->r.arrivalTime / TIME_PERIOD)) {
                    //Get candidate
                    if (candidate == -1) {
                        candidate = i;
                    }
                    else if(user[i].tail->r.arrivalTime < user[candidate].tail->r.arrivalTime) {
                        candidate = i;
                    }
                }   
            }
        }

        //Check user queue
        int empty = 1;
        for (i = 0; i < NUM_OF_USER; i++) {
            if (user[i].tail != NULL) {
                empty = 0;
            }
        }

        //Get candidate
        if (candidate != -1) {
            return candidate;
        }
        else {
            if (empty == 1)
                return -1;  //Finish
            else
                return -2;  //No any user selected in this period. Redo in the next period
        }
    #endif
}

/**
 * [印出所有user的credit]
 */
void printCredit() {
    int i;
    for(i = 0; i < NUM_OF_USER; i++) {
        printf(COLOR_GB" [CREDIT]USER_CREDIT_%u:%lf\n"COLOR_N, i+1, userCredit[i]);
    }
}