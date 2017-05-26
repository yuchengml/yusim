#include "yu_debug.h"

/**
 * [印出字串]
 * @param {char*} str [顯示字串]
 */
void PrintSomething(char* str) {
    printf(COLOR_YB"[DEBUG]%s\n"COLOR_N, str);
}

/**
 * [印出字串與錯誤碼並結束程式]
 * @param {int} rc  [錯誤碼]
 * @param {char*} str [顯示字串]
 */
void PrintError(int rc, char* str) {
	printf(COLOR_RB"[ERROR]%s%d\n"COLOR_N, str, rc);
	exit(1);
}

/**
 * [印出字串與錯誤碼]
 * @param {int} rc  [錯誤碼]
 * @param {char*} str [顯示字串]
 */
void PrintDebug(int rc, char* str) {
	printf(COLOR_YB"[DEBUG]%s%d\n"COLOR_N, str, rc);
}

/**
 * [印出Request資訊]
 * @param r   [Request]
 * @param {char*} str [顯示字串]
 */
void PrintREQ(REQ *r, char* str) {
	printf("[DEBUG][%s]:arrivalTime=%lf\n", str, r->arrivalTime);
	printf("[DEBUG][%s]:devno=%u\n", str, r->devno);
	printf("[DEBUG][%s]:diskBlkno=%8lu\n", str, r->diskBlkno);
	printf("[DEBUG][%s]:reqSize=%u\n", str, r->reqSize);
	printf("[DEBUG][%s]:reqFlag=%u\n", str, r->reqFlag);
	printf("[DEBUG][%s]:userno=%u\n", str, r->userno);
	printf("[DEBUG][%s]:responseTime=%lf\n", str, r->responseTime);
}