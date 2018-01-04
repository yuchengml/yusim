#include "yu_statistics.h"

/*USER STATISTICS INITIALIZATION*/
/**
 * [初始化User Statistics]
 */
void initUSERSTAT() {
    sysst.totalReq = 0;
    sysst.ssdReq = 0;
    sysst.totalUserReq = 0;
    sysst.UserReqInPeriod = 0;
    sysst.UserRReq = 0;
    sysst.totalSysReq = 0;
    sysst.evictCount = 0;
    sysst.dirtyCount = 0;
    sysst.hitCount = 0;
    sysst.missCount = 0;
    sysst.resTime = 0;
    sysst.resTimeInPeriod = 0;
    sysst.cachingSpace = 0;
    unsigned long i;
    for (i = 0; i < NUM_OF_USER; i++) {
        userst[i].totalReq = 0;
        userst[i].ssdReq = 0;
        userst[i].totalUserReq = 0;
        userst[i].UserReqInPeriod = 0;
        userst[i].UserRReq = 0;
        userst[i].totalSysReq = 0;
        userst[i].evictCount = 0;
        userst[i].dirtyCount = 0;
        userst[i].hitCount = 0;
        userst[i].missCount = 0;
        userst[i].resTime = 0;
        userst[i].resTimeInPeriod = 0;
        userst[i].cachingSpace = 0;
    }
}

/**
 * [印出所有user的Statistics]
 * @param {double} time [執行時間，為模擬系統時間]
 */
void printUSERSTAT(double time) {
    double response;
    printf(COLOR_BB"[SYSSTAT] Scheduling Time=%lf\n"COLOR_N, time);
    printf(COLOR_BB"[SYSSTAT] Total Page Requests(SSD/HDD):   %lu(%lu/%lu)\n"COLOR_N, sysst.totalReq, sysst.ssdReq, sysst.totalReq-sysst.ssdReq);
    printf(COLOR_BB"[SYSSTAT] Total User Requests(R/W):       %lu(%lu/%lu)\n"COLOR_N, sysst.totalUserReq, sysst.UserRReq, sysst.totalUserReq-sysst.UserRReq);
    printf(COLOR_BB"[SYSSTAT] Total System Requests:          %lu\n"COLOR_N, sysst.totalSysReq);
    printf(COLOR_BB"[SYSSTAT] Count of Eviction(Dirty):       %lu(%lu)\n"COLOR_N, sysst.evictCount, sysst.dirtyCount);
    printf(COLOR_BB"[SYSSTAT] Hit rate(Hit/Miss):             %lf(%lu/%lu)\n"COLOR_N, (double)sysst.hitCount/(double)(sysst.hitCount+sysst.missCount), sysst.hitCount, sysst.missCount);
    response = sysst.resTime/sysst.totalUserReq;
    printf(COLOR_BB"[SYSSTAT] Avg. Response Time:             %lf\n"COLOR_N, sysst.resTime/sysst.totalUserReq);
    printf(COLOR_BB"[SYSSTAT] Throughput:                     %lf(IOPS)\n"COLOR_N, 1000*(double)sysst.totalUserReq/time);
    response = sysst.resTimeInPeriod/sysst.UserReqInPeriod;
    printf(COLOR_BB"[SYSSTAT] Avg. Response Time(periods):    %lf\n"COLOR_N, response);
    printf(COLOR_BB"[SYSSTAT] Throughput(periods):            %lf(IOPS)\n"COLOR_N, 1000*(double)sysst.UserReqInPeriod/STAT_FOR_TIME_PERIODS/TIME_PERIOD);
    
    unsigned i;
    for (i = 0; i < NUM_OF_USER; i++) {
        printf(COLOR_BB"[USER_%u] Total Page Requests(SSD/HDD):   %lu(%lu/%lu)\n"COLOR_N, i+1, userst[i].totalReq, userst[i].ssdReq, userst[i].totalReq-userst[i].ssdReq);
        printf(COLOR_BB"[USER_%u] Total User Requests(R/W):       %lu(%lu/%lu)\n"COLOR_N, i+1, userst[i].totalUserReq, userst[i].UserRReq, userst[i].totalUserReq-userst[i].UserRReq);
        printf(COLOR_BB"[USER_%u] Total System Requests:          %lu\n"COLOR_N, i+1, userst[i].totalSysReq);
        printf(COLOR_BB"[USER_%u] Count of Eviction(Dirty):       %lu(%lu)\n"COLOR_N, i+1, userst[i].evictCount, userst[i].dirtyCount);
        printf(COLOR_BB"[USER_%u] Hit rate(Hit/Miss):             %lf(%lu/%lu)\n"COLOR_N, i+1, (double)userst[i].hitCount/(double)(userst[i].hitCount+userst[i].missCount), userst[i].hitCount, userst[i].missCount);
        response = userst[i].resTime/userst[i].totalUserReq;
        printf(COLOR_BB"[USER_%u] Avg. Response Time:             %lf\n"COLOR_N, i+1, response);
        printf(COLOR_BB"[USER_%u] Throughput:                     %lf(IOPS)\n"COLOR_N, i+1, 1000*(double)userst[i].totalUserReq/time);
        response = userst[i].resTimeInPeriod/userst[i].UserReqInPeriod;
        printf(COLOR_BB"[USER_%u] Avg. Response Time(periods):    %lf\n"COLOR_N, i+1, response);
        printf(COLOR_BB"[USER_%u] Throughput(periods):            %lf(IOPS)\n"COLOR_N, i+1, 1000*(double)userst[i].UserReqInPeriod/STAT_FOR_TIME_PERIODS/TIME_PERIOD);
        printf(COLOR_BB"[USER_%u] Caching Space:                  %lf\n"COLOR_N, i+1, userst[i].cachingSpace);
    }
}

/**
 * [寫檔至 Statistics File]
 * @param {unsigned long} periods [第?個時間段]
 * @param {FILE*} st [寫檔Pointer]
 */
void writeStatFile(unsigned long periods, FILE **st) {
    statPeriods--;
    if (statPeriods == 0) {
        double response, throughput;
        response = sysst.resTimeInPeriod/sysst.UserReqInPeriod;
        //Periods//Avg. Response Time//Throughput//Hit rate
        fprintf(*st, "%lu %lf %lf %lf", periods, response, 1000*(double)sysst.UserReqInPeriod/STAT_FOR_TIME_PERIODS/TIME_PERIOD, (double)sysst.hitCount/(double)(sysst.hitCount+sysst.missCount));
        sysst.resTimeInPeriod = 0;
        sysst.UserReqInPeriod = 0;
        unsigned i;
        for (i = 0; i < NUM_OF_USER; i++) {
            if (userst[i].UserReqInPeriod == 0) {
                response = 0;
                throughput = 0;
            }
            else {
                response = userst[i].resTimeInPeriod/userst[i].UserReqInPeriod;
                if (response == 0)
                    throughput = 0;
                else
                    throughput = 1000*(double)userst[i].UserReqInPeriod/STAT_FOR_TIME_PERIODS/TIME_PERIOD;
            }
            //Avg. Response Time//Throughput//Hit rate//Caching Space
            fprintf(*st, " %lf %lf %lf %lf", response, throughput, (double)userst[i].hitCount/(double)(userst[i].hitCount+userst[i].missCount), userst[i].cachingSpace);
            //因為只記錄1個period，因此必須清空
            userst[i].resTimeInPeriod = 0;
            userst[i].UserReqInPeriod = 0;
        }
        fprintf(*st, "\n");

        statPeriods = STAT_FOR_TIME_PERIODS;
    }
}

/**
 * [印出yu_parameter.h資訊]
 */
void printParameters() {
    printf("[PARM]DISKSIM_SECTOR = %u\n", DISKSIM_SECTOR);
    printf("[PARM]SSD_PAGE_SIZE = %u\n", SSD_PAGE_SIZE);
    printf("[PARM]SSD_PAGE2SECTOR = %u\n", SSD_PAGE2SECTOR);
    printf("[PARM]SSD_PAGES_PER_BLOCK = %u\n", SSD_PAGES_PER_BLOCK);
    printf("[PARM]SSD_BLOCK_SIZE = %u\n", SSD_BLOCK_SIZE);
    printf("[PARM]TIME_PERIOD = %u\n", TIME_PERIOD);
    printf("[PARM]NUM_OF_USER = %u\n", NUM_OF_USER);
    printf("[PARM]SSD_CACHING_SPACE_BY_PAGES = %u\n", SSD_CACHING_SPACE_BY_PAGES);
    printf("[PARM]SSD_N_ELEMENTS = %u\n", SSD_N_ELEMENTS);
}

/**
 * [寫檔至 Result File]
 * @param {FILE*} st [寫檔Pointer]
 */
void writeResultFile(FILE **result, double time) {
    fprintf(*result, "[PARM]DISKSIM_SECTOR = %u\n", DISKSIM_SECTOR);
    fprintf(*result, "[PARM]SSD_PAGE_SIZE = %u\n", SSD_PAGE_SIZE);
    fprintf(*result, "[PARM]SSD_PAGE2SECTOR = %u\n", SSD_PAGE2SECTOR);
    fprintf(*result, "[PARM]SSD_PAGES_PER_BLOCK = %u\n", SSD_PAGES_PER_BLOCK);
    fprintf(*result, "[PARM]SSD_BLOCK_SIZE = %u\n", SSD_BLOCK_SIZE);
    fprintf(*result, "[PARM]TIME_PERIOD = %u\n", TIME_PERIOD);
    fprintf(*result, "[PARM]NUM_OF_USER = %u\n", NUM_OF_USER);
    fprintf(*result, "[PARM]SSD_CACHING_SPACE_BY_PAGES = %u\n", SSD_CACHING_SPACE_BY_PAGES);
    fprintf(*result, "[PARM]SSD_N_ELEMENTS = %u\n", SSD_N_ELEMENTS);

    double response;
    fprintf(*result, "[SYSSTAT] Scheduling Time=%lf\n", time);
    fprintf(*result, "[SYSSTAT] Total Page Requests(SSD/HDD):   %lu(%lu/%lu)\n", sysst.totalReq, sysst.ssdReq, sysst.totalReq-sysst.ssdReq);
    fprintf(*result, "[SYSSTAT] Total User Requests(R/W):       %lu(%lu/%lu)\n", sysst.totalUserReq, sysst.UserRReq, sysst.totalUserReq-sysst.UserRReq);
    fprintf(*result, "[SYSSTAT] Total System Requests:          %lu\n", sysst.totalSysReq);
    fprintf(*result, "[SYSSTAT] Count of Eviction(Dirty):       %lu(%lu)\n", sysst.evictCount, sysst.dirtyCount);
    fprintf(*result, "[SYSSTAT] Hit rate(Hit/Miss):             %lf(%lu/%lu)\n", (double)sysst.hitCount/(double)(sysst.hitCount+sysst.missCount), sysst.hitCount, sysst.missCount);
    response = sysst.resTime/sysst.totalUserReq;
    fprintf(*result, "[SYSSTAT] Avg. Response Time:             %lf\n", response);
    fprintf(*result, "[SYSSTAT] Throughput:                     %lf(IOPS)\n", 1000*(double)sysst.totalUserReq/time);
    fprintf(*result, "[SYSSTAT] Caching Space:                  %lf\n", sysst.cachingSpace);

    unsigned i;
    for (i = 0; i < NUM_OF_USER; i++) {
        fprintf(*result, "[USER_%u] Total Page Requests(SSD/HDD):   %lu(%lu/%lu)\n", i+1, userst[i].totalReq, userst[i].ssdReq, userst[i].totalReq-userst[i].ssdReq);
        fprintf(*result, "[USER_%u] Total User Requests(R/W):       %lu(%lu/%lu)\n", i+1, userst[i].totalUserReq, userst[i].UserRReq, userst[i].totalUserReq-userst[i].UserRReq);
        fprintf(*result, "[USER_%u] Total System Requests:          %lu\n", i+1, userst[i].totalSysReq);
        fprintf(*result, "[USER_%u] Count of Eviction(Dirty):       %lu(%lu)\n", i+1, userst[i].evictCount, userst[i].dirtyCount);
        fprintf(*result, "[USER_%u] Hit rate(Hit/Miss):             %lf(%lu/%lu)\n", i+1, (double)userst[i].hitCount/(double)(userst[i].hitCount+userst[i].missCount), userst[i].hitCount, userst[i].missCount);
        response = userst[i].resTime/userst[i].totalUserReq;
        fprintf(*result, "[USER_%u] Avg. Response Time:             %lf\n", i+1, response);
        fprintf(*result, "[USER_%u] Throughput:                     %lf(IOPS)\n", i+1, 1000*(double)userst[i].totalUserReq/time);
        fprintf(*result, "[USER_%u] Caching Space:                  %lf\n", i+1, userst[i].cachingSpace);
    }
}