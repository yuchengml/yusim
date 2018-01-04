#include <stdlib.h>
#include <stdio.h>
#define NUM_OF_USER 1
typedef struct req {
		double arrivalTime;		//抵達時間
		unsigned devno;			//裝置編號(預設為0)
		unsigned long diskBlkno;//Block編號(根據Disksim格式)
		unsigned reqSize;		//Block連續數量(至少為1)
		unsigned reqFlag;		//讀:1;寫:0
		unsigned userno;		//使用者編號(1~n)
		double responseTime;	//反應時間(初始為0)
	} REQ;

int main(int argc, char *argv[]){
	if (argc != 3) {
    	fprintf(stderr, "usage: %s <Input trace file> <Output trace file>\n", argv[0]);
    	exit(1);
    }
    FILE *input, *output;

    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
    
    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));

    while (!feof(input)) {
        fscanf(input, "%lf%u%lu%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag);
        fprintf(output, "%lf\t%u\t%lu\t%u\t%u\t%u\n", tmp->arrivalTime, tmp->devno, tmp->diskBlkno, tmp->reqSize, tmp->reqFlag, NUM_OF_USER);
    }

    printf("[Finish]\n");

    fclose(input);
    fclose(output);

	return 0;
}
