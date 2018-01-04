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
	if (argc != 5) {
    	fprintf(stderr, "usage: %s <New user number> <Input trace file 1> <Input trace file 2> <Output trace file>\n", argv[0]);
    	exit(1);
    }

    unsigned newUser;
    newUser = atoi(argv[1]);
 	printf("New user:%u\nSynthesizing.......", newUser);
 	
    FILE *input[2], *output;

    input[0] = fopen(argv[2], "r");
    input[1] = fopen(argv[3], "r");
    output = fopen(argv[4], "w");
    
    int i;
    for (i = 0; i < newUser-1; i++) {
    	fprintf(output, "%u ", 1);
    }
    fprintf(output, "%u\n", 1);
    
    REQ *tmp[2];
    tmp[0] = calloc(1, sizeof(REQ));
    tmp[1] = calloc(1, sizeof(REQ));

    int flag[2]={0,0};
    while(1) {
        if (flag[0] == 0) {
            if (!feof(input[0])) {
                fscanf(input[0], "%lf%u%lu%u%u%u", &tmp[0]->arrivalTime, &tmp[0]->devno, &tmp[0]->diskBlkno, &tmp[0]->reqSize, &tmp[0]->reqFlag, &tmp[0]->userno);
                flag[0] = 1;
            }
            else
                flag[0] = -1;
        }

        if (flag[1] == 0) {
            if (!feof(input[1])) {
                fscanf(input[1], "%lf%u%lu%u%u%u", &tmp[1]->arrivalTime, &tmp[1]->devno, &tmp[1]->diskBlkno, &tmp[1]->reqSize, &tmp[1]->reqFlag, &tmp[1]->userno);
                flag[1] = 1;
            }
            else
                flag[1] = -1;
        }
    	
        if (flag[0] == -1 && flag[1] == -1)
            break;

    	if (flag[0] == 1 && flag[1] == 1) {
            if (tmp[0]->arrivalTime <= tmp[1]->arrivalTime) {
                fprintf(output, "%lf\t%u\t%lu\t%u\t%u\t%u\n", tmp[0]->arrivalTime, tmp[0]->devno, tmp[0]->diskBlkno, tmp[0]->reqSize, tmp[0]->reqFlag, tmp[0]->userno);
                flag[0] = 0;
            }
    		else {
                fprintf(output, "%lf\t%u\t%lu\t%u\t%u\t%u\n", tmp[1]->arrivalTime, tmp[1]->devno, tmp[1]->diskBlkno, tmp[1]->reqSize, tmp[1]->reqFlag, newUser);
                flag[1] = 0;
            }
        }
        else if (flag[0] == 1 && flag[1] == -1) {
            fprintf(output, "%lf\t%u\t%lu\t%u\t%u\t%u\n", tmp[0]->arrivalTime, tmp[0]->devno, tmp[0]->diskBlkno, tmp[0]->reqSize, tmp[0]->reqFlag, tmp[0]->userno);
            flag[0] = 0;
        }
        else if (flag[0] == -1 && flag[1] == 1) {
            fprintf(output, "%lf\t%u\t%lu\t%u\t%u\t%u\n", tmp[1]->arrivalTime, tmp[1]->devno, tmp[1]->diskBlkno, tmp[1]->reqSize, tmp[1]->reqFlag, newUser);
            flag[1] = 0;
        }
    	else{
            printf("flag[0]:%d\n", flag[0]);
            printf("flag[1]:%d\n", flag[1]);
    		fprintf(stderr, "Error\n");
    		exit(1);
    	}
    }
    printf("[Finish]\n");

    fclose(input[0]);
    fclose(input[1]);
    fclose(output);

	return 0;
}
