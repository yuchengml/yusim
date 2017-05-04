#include "yu_credit.h"

/*CREDIT INITIALIZATION*/
/**[初始化Credit，根據User number]
 * @param {unsigned} userno [User number(0-n)]
 * @return {double} userCredit [Modified user credit]
 */
double creditInit(unsigned userno) {
    userCredit[userno] = INIT_CREDIT;
    return userCredit[userno];
}

/*CREDIT REPLENISHMENT*/
/**
 * [補充Credit，根據User number]
 * @param {unsigned} userno [User number(0-n)]
 * @return {double} userCredit [Modified user credit]
 */
double creditReplenish(unsigned userno) {
    userCredit[userno] = INIT_CREDIT;
    return userCredit[userno];
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
        printf(COLOR_GB"  [CREDIT]USER_CREDIT_%u:%lf\n"COLOR_N, i, userCredit[i]);
    }
}