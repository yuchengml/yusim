#include "yu_ipc.h"

int createMessageQueue(key_t key, int msqflag) {
	int msqid = -1;
	msqid = msgget(key, msqflag | 0666);
	if(msqid >= 0) {
		//PrintDebug(msqid, "Message Queue Identifier:");
		return msqid;
	}
	else
		return -1;
}

int removeMessageQueue(key_t key, struct msqid_ds *msqds) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);
	if(msgctl(msqid, IPC_RMID, msqds) == 0) {
		//PrintDebug(msqid, "Remove message queue:");
		return msqid;
	}
	else
		return -1;
		
}

int sendRequestByMSQ(key_t key, REQ *r, long int msgtype) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF msg;
	msg.msgType = msgtype;
	msg.req.arrivalTime = r->arrivalTime;
    msg.req.devno = r->devno;
    msg.req.blkno = r->blkno;
    msg.req.reqSize = r->reqSize;
    msg.req.reqFlag = r->reqFlag;
    msg.req.userno = r->userno;
    msg.req.responseTime = r->responseTime;
	if(msgsnd(msqid, (void *)&msg, MSG_SIZE, 0) == 0) {
		//PrintDebug(msqid, "A request sent to MSQ:");
		return 0;
	}
	else
		return -1;
		//PrintError(msqid, "A request not sent to MSQ:");	
}

int recvRequestByMSQ(key_t key, REQ *r, long int msgtype) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF buf;
	if(msgrcv(msqid, (void *)&buf, MSG_SIZE, msgtype, 0) >= 0) {/*return bytes sent*/
		//PrintDebug(msqid, "A request received from MSQ:");
		r->arrivalTime = buf.req.arrivalTime;
		r->devno = buf.req.devno;
		r->blkno = buf.req.blkno;
		r->reqSize = buf.req.reqSize;
		r->reqFlag = buf.req.reqFlag;
		r->userno = buf.req.userno;
		r->responseTime = buf.req.responseTime;
		return 0;
	}
	else {
		//PrintError(msqid, "A request not received from MSQ in recvRequestByMSQ():");
		return -1;
	}	
}

void testMessageQueue() {
	//data structure for msg queue
	struct msqid_ds ds;
	if(createMessageQueue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1)
		PrintError(-1, " MSQ create error in createMessageQueue():");

	REQ *r;
	r = calloc(1, sizeof(REQ));
	r->arrivalTime = 0.00;
	r->devno = 0;
	r->blkno = 1234;
	r->reqSize = 16;
	r->reqFlag = 0;
	r->userno = 0;
	r->responseTime = -1;

	PrintREQ(r ,"To sddsim in testMessageQueue()");

	if(sendRequestByMSQ(KEY_MSQ_DISKSIM_1, r, MSG_TYPE_DISKSIM_1) == -1) {
	    PrintError(-1, "A request not sent to MSQ in sendRequestByMSQ() return:");
	}

	REQ *rp = NULL;
	rp = calloc(1, sizeof(REQ));
	if(recvRequestByMSQ(KEY_MSQ_DISKSIM_1, rp, MSG_TYPE_DISKSIM_1) == -1)
	    PrintError(-1, "A request not received from MSQ in recvRequestByMSQ():");
	
	PrintREQ(rp ,"from sddsim in testMessageQueue()");
	
	if(removeMessageQueue(KEY_MSQ_DISKSIM_1, &ds) == -1)
		PrintError(KEY_MSQ_DISKSIM_1, "Not remove message queue:(key)");
	
	if(createMessageQueue(KEY_MSQ_DISKSIM_2, IPC_CREAT) == -1)
		PrintError(-1, " MSQ create error in createMessageQueue() return:");
	if(removeMessageQueue(KEY_MSQ_DISKSIM_2, &ds) == -1)
		PrintError(KEY_MSQ_DISKSIM_2, "Not remove message queue:(key)");
}

int sendFinishControl(key_t key, long int msgtype) {
	REQ *ctrl;
    ctrl = calloc(1, sizeof(REQ));
    ctrl->reqFlag = MSG_REQUEST_CONTROL_FLAG_FINISH;
    if(sendRequestByMSQ(key, ctrl, msgtype) == -1) {
        PrintError(key, "A control request not sent to MSQ in sendRequestByMSQ() return:");
        return -1;
    }
    else
    	return 0;
}

int sendSimulateControl(key_t key, long int msgtype) {
	REQ *ctrl;
	ctrl = calloc(1, sizeof(REQ));
	ctrl->reqFlag = MSG_REQUEST_CONTROL_FLAG_SIMULATE;
	if(sendRequestByMSQ(key, ctrl, msgtype) == -1) {
	    PrintError(key, "A control request not sent to MSQ in sendRequestByMSQ() return:");
		return -1;
	}
	else
		return 0;
}