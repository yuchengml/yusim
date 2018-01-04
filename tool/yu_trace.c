#include "yu_trace.h"

void traceFile2workingSet(FILE *fin) {
    unsigned long maxBlkno = 0;
    unsigned long totalRequest = 0, readReqCnt = 0, writeReqCnt = 0, unknownReqCnt = 0;
    unsigned long totalPage = 0, currentPage = 0, workingPage = 0;
    int i, page_count;
    unsigned long blkno;
    TAB *set=NULL;
    TAB *search;
    REQ *tmp;
    tmp = calloc(1, sizeof(REQ));

    while(!feof(fin)) {
        tmp->diskBlkno = 0;
        tmp->reqSize = 0;
        fscanf(fin, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);
        totalPage += (tmp->reqSize/SSD_PAGE2SECTOR);
        totalRequest++;

        if (tmp->reqSize >0 && maxBlkno < tmp->diskBlkno + tmp->reqSize - 1)
            maxBlkno = tmp->diskBlkno + tmp->reqSize - 1;

        if (tmp->reqFlag == 1)
            readReqCnt++;
        else if (tmp->reqFlag == 0)
            writeReqCnt++;
        else
            unknownReqCnt++;
    }

    printf("Total Request: %lu\n", totalRequest);
    printf("Total Page: %lu\n", totalPage);
    printf("Avg. Request Size: %lf (pages)\n", (double)totalPage/(double)totalRequest);
    printf("Write Ratio: %lf%% (Unknown:%lu)\n", 100*(double)writeReqCnt/((double)writeReqCnt+(double)readReqCnt), unknownReqCnt);
    printf("Max. Accessed DiskBlkno: %lu (Next new blkno should be %lu)\n", maxBlkno, maxBlkno+1);

    rewind(fin);

    while(!feof(fin)) {
        fscanf(fin, "%lf%u%lu%u%u%u", &tmp->arrivalTime, &tmp->devno, &tmp->diskBlkno, &tmp->reqSize, &tmp->reqFlag, &tmp->userno);
    
        page_count = tmp->reqSize/SSD_PAGE2SECTOR;
        for (i = 0; i < page_count; i++) {
            blkno = tmp->diskBlkno + i*SSD_PAGE2SECTOR;
            for (search = set; search != NULL; search = search->next) {
                if (search->diskBlkno == blkno)
                    break;
            }

            if (search == NULL){
                TAB *new;
                new = calloc(1, sizeof(TAB));
                new->diskBlkno = blkno;
                new->next = set;
                set = new;
                workingPage++;
            }
            currentPage++;
        }
        printProgress(currentPage, totalPage);
    }
    printf("\n");//For printProgress()
    printf("Working set pages: %lu\n", workingPage);
}


void printProgress(unsigned long current, unsigned long total) {
    printf ("\rProgress:%9lu / %9lu", current, total);
    fflush (stdout);
}

int main(int argc, char const *argv[]) {
	printf("This is a file to trace workload.\n");

    // File IO describer
    FILE *fin;

    double arrivalTime;
    unsigned devno, blkno, reqSize, reqFlag, userno;

    fin = fopen(argv[1], "r");
    //fout = fopen(argv[2], "w");
 
    if (argc != 3) {
        printf("Used:%s inputTrace (1/0):weight or not\n", argv[0]);
        exit(1);
    }
 
    if (!fin) {
        printf("Input file open error...\n");
        exit(1);
    }
    
    if(!strcmp(argv[2], "1")){
        int i;
        unsigned weight = 0;
        for(i = 0; i < NUM_OF_USER; i++) {
            fscanf(fin, "%u", &weight);
            printf("User Weight:%u\n", weight);
        }
    }
    
    traceFile2workingSet(fin);

    fclose(fin);
    //fclose(fout);
    return 0;
}
