#include "yu_statistics.h"

/*USER STATISTICS INITIALIZATION*/
/**
 * [初始化User Statistics]
 */
void initUSERSTAT() {
    unsigned long i;
    for (i = 0; i < NUM_OF_USER; i++) {
        userst[i].totalBlkReq = 0;
        userst[i].ssdBlkReq = 0;
        userst[i].totalUserReq = 0;
        userst[i].UserRReq = 0;
        userst[i].totalSysReq = 0;
        userst[i].evictCount = 0;
        userst[i].dirtyCount = 0;
        userst[i].hitCount = 0;
        userst[i].missCount = 0;
        userst[i].responseTime = 0;
        userst[i].cachingSpace = 0;
    }
}

/**
 * [印出所有user的Statistics]
 * @param {double} time [執行時間，為模擬系統時間]
 */
void printUSERSTAT(double time) {
    printf(COLOR_BB"[USERSTAT] Scheduling Time=%lf\n"COLOR_N, time);
    unsigned long i;
    for (i = 0; i < NUM_OF_USER; i++) {
        printf(COLOR_BB"[USER_%lu] Total Block Requests(SSD/HDD):%lu(%lu/%lu)\n"COLOR_N, i+1, userst[i].totalBlkReq, userst[i].ssdBlkReq, userst[i].totalBlkReq-userst[i].ssdBlkReq);
        printf(COLOR_BB"[USER_%lu] Total User Requests(R/W):     %lu(%lu/%lu)\n"COLOR_N, i+1, userst[i].totalUserReq, userst[i].UserRReq, userst[i].totalUserReq-userst[i].UserRReq);
        printf(COLOR_BB"[USER_%lu] Total System Requests:        %lu\n"COLOR_N, i+1, userst[i].totalSysReq);
        printf(COLOR_BB"[USER_%lu] Count of Eviction(Dirty):     %lu(%lu)\n"COLOR_N, i+1, userst[i].evictCount, userst[i].dirtyCount);
        printf(COLOR_BB"[USER_%lu] Hit rate(Hit/Miss):           %lf(%lu/%lu)\n"COLOR_N, i+1, (double)userst[i].hitCount/(double)(userst[i].hitCount+userst[i].missCount), userst[i].hitCount, userst[i].missCount);
        printf(COLOR_BB"[USER_%lu] Throughput:                   %lf(IOPS)\n"COLOR_N, i+1, (double)userst[i].totalUserReq/time);
        printf(COLOR_BB"[USER_%lu] Response Time:                %lf\n"COLOR_N, i+1, userst[i].responseTime);
        printf(COLOR_BB"[USER_%lu] Caching Space:                %lf\n"COLOR_N, i+1, userst[i].cachingSpace);
    }
}

/**
 * [寫檔至 Statistics File]
 * @param {double} time [執行時間，為模擬系統時間]
 * @param {FILE*} st [寫檔Pointer]
 */
void writeStatFile(double time, FILE **st) {
    fprintf(*st, "%lu\n", (unsigned long)floor(time / TIME_PERIOD));
    unsigned long i;
    for (i = 0; i < NUM_OF_USER; i++) {
        // User num, Response Time, Throughput, Hit rate, Caching Space
        fprintf(*st, "%lu %lf %lf %lf %lf\n", i+1, userst[i].responseTime, (double)userst[i].totalUserReq/time, (double)userst[i].hitCount/(double)(userst[i].hitCount+userst[i].missCount), userst[i].cachingSpace);
    }
}

/**
 * [印出yu_parameter.h資訊]
 */
void printParameters() {
    printf("DISKSIM_SECTOR = %u\n", DISKSIM_SECTOR);
    printf("SSD_PAGE_SIZE = %u\n", SSD_PAGE_SIZE);
    printf("SSD_PAGE2SECTOR = %u\n", SSD_PAGE2SECTOR);
    printf("SSD_PAGES_PER_BLOCK = %u\n", SSD_PAGES_PER_BLOCK);
    printf("SSD_BLOCK_SIZE = %u\n", SSD_BLOCK_SIZE);
    printf("SSD_BLOCK2SECTOR = %u\n", SSD_BLOCK2SECTOR);
    printf("HDD_BLOCK_SIZE = %u\n", HDD_BLOCK_SIZE);
    printf("HDD_BLOCK2SECTOR = %u\n", HDD_BLOCK2SECTOR);
    printf("TIME_PERIOD = %u\n", TIME_PERIOD);
    printf("NUM_OF_USER = %u\n", NUM_OF_USER);
    printf("SSD_CACHING_SPACE_BY_PAGES = %u\n", SSD_CACHING_SPACE_BY_PAGES);
}