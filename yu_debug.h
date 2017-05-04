#ifndef YU_DEBUG_H
#define YU_DEBUG_H
#include <stdlib.h>
#include <stdio.h>
#include "yu_structure.h"

	/*印出字串*/
	void PrintSomething(char* str);
	/*印出字串與錯誤碼並結束程式*/
	void PrintError(int rc, char* str);
	/*印出字串與錯誤碼*/
	void PrintDebug(int rc, char* str);
	/*印出Request資訊*/
	void PrintREQ(REQ *r, char* str);
#endif
