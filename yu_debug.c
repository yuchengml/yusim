#include "yu_debug.h"

void PrintSomething(char* str) {
    printf(COLOR_YB"[DEBUG]%s\n"COLOR_N, str);
}

void PrintError(int rc, char* str) {
	printf(COLOR_RB"[ERROR]%s%d\n"COLOR_N, str, rc);
	exit(1);
}

void PrintDebug(int rc, char* str) {
	printf(COLOR_YB"[DEBUG]%s%d\n"COLOR_N, str, rc);
}

void PrintREQ(REQ *r, char* str) {
	printf("[DEBUG][%s]:arrivalTime=%lf\n", str, r->arrivalTime);
	printf("[DEBUG][%s]:devno=%u\n", str, r->devno);
	printf("[DEBUG][%s]:blkno=%8lu\n", str, r->blkno);
	printf("[DEBUG][%s]:reqSize=%u\n", str, r->reqSize);
	printf("[DEBUG][%s]:reqFlag=%u\n", str, r->reqFlag);
	printf("[DEBUG][%s]:userno=%u\n", str, r->userno);
	printf("[DEBUG][%s]:responseTime=%lf\n", str, r->responseTime);
}