#ifndef YU_IPC_H
#define YU_IPC_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "yu_debug.h"
#include "yu_structure.h"

	
	typedef struct msgbuf {
		long int msgType;
		REQ req;
	} MSGBUF;

	#define MSG_SIZE sizeof(MSGBUF)-sizeof(long int)

	int createMessageQueue(key_t key, int msqflag);
	int removeMessageQueue(key_t key, struct msqid_ds *msqds);
	int sendRequestByMSQ(key_t key, REQ *r, long int msgtype);
	int recvRequestByMSQ(key_t key, REQ *r, long int msgtype);
	/*******
	EXAMPLE:
	if(sendRequestByMSQ(KEY_MSQ_DISKSIM_2, tmp, MSG_TYPE_DISKSIM_2) == -1)
	    PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");
	sendSimulateControl(KEY_MSQ_DISKSIM_2, MSG_TYPE_DISKSIM_2);

	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_2, rtn, MSG_TYPE_DISKSIM_2_SERVED) == -1)
	    PrintError(-1, "A served request not received from MSQ in recvRequestByMSQ():");
	*******/
	void testMessageQueue();

	//sendFinishControl() in rmDisksim()
	int sendFinishControl(key_t key, long int msgtype);
	int sendSimulateControl(key_t key, long int msgtype);
#endif
