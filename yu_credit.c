#include "yu_credit.h"

/*CREDIT INITIALIZATION*/
/**[針對所有Users初始化Credit]
 * @return {int} 0/-1 [success/fail]
 */
int creditInit() {
    if (totalWeight == 0) {
        PrintError(totalWeight, "[CREDIT]You need to call initUserCACHE() first due to error totalWeight = ");
        return -1;
    }

    int i;
    for(i = 0; i < NUM_OF_USER; i++) {
        userCredit[i] = INIT_CREDIT * ((double)userWeight[i]/totalWeight);
    }

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
    if (totalWeight == 0) {
        PrintError(totalWeight, "[CREDIT]Error totalWeight = ");
        return -1;
    }
    
    int i;
    for(i = 0; i < NUM_OF_USER; i++) {
        if (userCredit[i] < 0)
            userCredit[i] += INIT_CREDIT * ((double)userWeight[i]/totalWeight); //"+=" 考慮上次可能為負
        else
            userCredit[i] = INIT_CREDIT * ((double)userWeight[i]/totalWeight);
    }
    
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
    userCredit[userno] += value;
    return userCredit[userno];
}

/*CREDIT-BASED SCHEDULER*/
/**
 * [根據Credit策略，判斷哪個user允許服務其request]
 * @param {USER_QUE} user[] [ All of user queues]
 * @return {int} candidate [下一個候選user，作為下一個request的依據]
 */
int creditScheduler(USER_QUE user[]) {
    int i, candidate=-1;
    double max=0;
    //找有最大credit且有request等待的user
    for (i = 0; i < NUM_OF_USER; i++) {
        if (user[i].tail != NULL) {
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

    if (candidate != -1) {
        return candidate;
    }

    //找有request等待的user
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
    return candidate;
}

/**
 * [印出所有user的credit]
 */
void printCredit() {
    int i;
    for(i = 0; i < NUM_OF_USER; i++) {
        printf(COLOR_GB" [CREDIT]USER_CREDIT_%u:%lf\n"COLOR_N, i, userCredit[i]);
    }
}