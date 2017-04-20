#include "yu_credit.h"

credit creditInit(USER_QUE *user) {
    user->credit = INIT_CREDIT;
    return user->credit;
}

credit creditReplenish(USER_QUE *user) {
    user->credit = INIT_CREDIT;
    return user->credit;
}

credit creditCharge(USER_QUE *user, double value) {
    user->credit -= value;
    return user->credit;
}

credit creditCompensate(USER_QUE *user, double value) {
    user->credit += value;
    return user->credit;
}

int creditScheduler(USER_QUE user[]) {
    int i, candidate=-1;
    credit max=0;
    //找有最大credit且有request等待的user
    for (i = 0; i < NUM_OF_USER; i++) {
        if (user[i].tail != NULL) {
            if (user[i].credit > max) {
                max = user[i].credit;
                candidate = i;
            }
            else if (user[i].credit == max && max != 0){
                if(user[i].tail->r.arrivalTime < user[candidate].tail->r.arrivalTime) {
                    max = user[i].credit;
                    candidate = i;
                }
            }
        }
        
    }

    if (candidate != -1) {
        return candidate;
    }

    //找有request等待的user
    candidate = -1;
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