#include "yu_trace.h"

void traceFile2print(FILE *fin) {
    double arrivalTime;
    unsigned devno, blkno, reqSize, reqFlag, userno;
	while(!feof(fin)) {
        fscanf(fin, "%lf%u%u%u%u%u", &arrivalTime, &devno, &blkno, &reqSize, &reqFlag, &userno);
    	fprintf(stderr, "arrivalTime=%lf\ndevno=      %u\nblkno=      %u\nreqSize=    %u\nreqFlag=    %u\nuserno=     %u\n", arrivalTime, devno, blkno, reqSize, reqFlag, userno);
    	getchar();
    }
}

void traceFile2max(FILE *fin) {
    unsigned long maxBlkno = 0;
    double arrivalTime;
    unsigned long devno, blkno, reqSize, reqFlag, userno;
    while(!feof(fin)) {
        fscanf(fin, "%lf%lu%lu%lu%lu%lu", &arrivalTime, &devno, &blkno, &reqSize, &reqFlag, &userno);
        if (maxBlkno < blkno + reqSize - 1)
            maxBlkno = blkno + reqSize - 1;
        //fprintf(stderr, "arrivalTime=%lf\ndevno=      %u\nblkno=      %u\nreqSize=    %u\nreqFlag=    %u\nuserno=     %u\n", arrivalTime, devno, blkno, reqSize, reqFlag, userno);
        //getchar();
    }
    printf("maxBlkno = %lu\n", maxBlkno);
}

void traceFile2blkno(FILE *fin) {
    unsigned long num = 0;
    double arrivalTime;
    unsigned long devno, blkno, reqSize, reqFlag, userno;
    /*block_count代表此request共存取多少SSD Block*/
    int block_count;
    while(!feof(fin)) {
        fscanf(fin, "%lf%lu%lu%lu%lu%lu", &arrivalTime, &devno, &blkno, &reqSize, &reqFlag, &userno);
        block_count = reqSize/SSD_PAGE2SECTOR;
        int i;
        for (i = 0; i < block_count; i++) {
            num++;
        }
    }
    printf("num = %lu\n", num);
}

int main(int argc, char const *argv[]) {
	printf("This is a file to trace workload.\n");

    // File IO describer
    FILE *fin;

    double arrivalTime;
    unsigned devno, blkno, reqSize, reqFlag, userno;

    fin = fopen(argv[1], "r");
    //fout = fopen(argv[2], "w");
 
    if (argc != 2) {
        printf("Used:%s inputTrace\n", argv[0]);
        exit(1);
    }
 
    if (!fin) {
        printf("Input file open error...\n");
        exit(1);
    }
    
    //if (!fout) {
    //    printf("Output file open error...\n");
    //    exit(1);
    //}
    //
    int i;
    unsigned weight = 0;
    for(i = 0; i < NUM_OF_USER; i++) {
        fscanf(fin, "%u", &weight);
        //userWeight[i] = weight;
    }
    

    //traceFile2print(fin);
    traceFile2max(fin);
    //traceFile2blkno(fin);

    fclose(fin);
    //fclose(fout);
    return 0;
}
