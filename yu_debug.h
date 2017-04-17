#ifndef YU_DEBUG_H
#define YU_DEBUG_H
#include <stdlib.h>
#include <stdio.h>

#include "yu_structure.h"

	void PrintSomething(char* str);
	void PrintError(int rc, char* str);
	void PrintDebug(int rc, char* str);
	void PrintREQ(REQ *r, char* str);
#endif
