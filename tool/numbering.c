#include <stdlib.h>
#include <stdio.h>

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
	if (argc != 4) {
    	fprintf(stderr, "usage: %s <Input trace file> <Output trace file> <Start Blkno>\n", argv[0]);
    	exit(1);
    }

    unsigned long startBlk=0;
    startBlk = (unsigned long)atol(argv[3]);

 	printf("Analyzing.......");

    unsigned long minBlk=0, maxBlk=0, readCnt=0, writeCnt=0, unknownFlag=0;
    double first_time=0;
 	
    FILE *input, *output;

    input = fopen(argv[1], "r");
    
    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));

    if (!feof(input)) {
        fscanf(input, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);

        minBlk = tmp->diskBlkno;
        maxBlk = tmp->diskBlkno;
        first_time = tmp->arrivalTime;

        if (tmp->reqFlag == 1)
            readCnt++;
        else if (tmp->reqFlag == 0)
            writeCnt++;
        else
            unknownFlag++;
    }


    while (!feof(input)) {
    	fscanf(input, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);

        if (tmp->diskBlkno < minBlk)
        {
            minBlk = tmp->diskBlkno;
        }
        
        if (tmp->reqSize > 0 && tmp->diskBlkno+tmp->reqSize-8 > maxBlk)
        {
            maxBlk = tmp->diskBlkno+tmp->reqSize-8;
            printf("blkno:%lu , size:%u\n", tmp->diskBlkno, tmp->reqSize);
        }


        if (tmp->reqFlag == 1)
            readCnt++;
        else if (tmp->reqFlag == 0)
            writeCnt++;
        else
            unknownFlag++;
    	
    }
    printf("[Finish]\n");

    printf("minBlk:%lu, maxBlk:%lu, readCnt:%lu, writeCnt:%lu, unknownFlag:%lu\n", minBlk, maxBlk, readCnt, writeCnt, unknownFlag);

    if (minBlk == 0 && startBlk == 0)
    {
        printf("Already done!\n");
        fclose(input);
        return 0;
    }
    else if (first_time != 0) {
        printf("Blkno Already done but Time!\n");
    }

    rewind(input);

    output = fopen(argv[2], "w");

    printf("Re-numbering.......");
    while (!feof(input)) {
        fscanf(input, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);
        fprintf(output, "%lf\t%u\t%lu\t%u\t%u\t%u\n", tmp->arrivalTime-first_time, tmp->devno, tmp->diskBlkno-minBlk+startBlk, tmp->reqSize, tmp->reqFlag, tmp->userno);
    }

    printf("[Finish]\n");

    fclose(input);
    fclose(output);

	return 0;
}
