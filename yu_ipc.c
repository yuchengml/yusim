#include "yu_ipc.h"

/*CREATE MESSAGE QUEUE*/
/**
 * [根據指定的Key值和Message Queue Flag建立]
 * @param {key_t} key [Key值]
 * @param {int} msqflag [Message Queue Flag]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
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

/*REMOVE MESSAGE QUEUE*/
/**
 * [根據指定的Key值和Message Queue Flag刪除]
 * @param {key_t} key [Key值]
 * @param {struct msqid_ds*} msqds [Message Queue Data Structure]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
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

/*SEND REQUEST BY MESSAGE QUEUE*/
/**
 * [根據指定的Key值代表特定的Message Queue和Message Type傳送Request]
 * @param {key_t} key [Key值]
 * @param {REQ*} r [Request]
 * @param {long} msgtype [Message Queue Type]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int sendRequestByMSQ(key_t key, REQ *r, long msgtype) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF msg;
	msg.msgType = msgtype;
	msg.req.arrivalTime = r->arrivalTime;
    msg.req.devno = r->devno;
    msg.req.diskBlkno = r->diskBlkno;
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
}

/*RECEIVE REQUEST BY MESSAGE QUEUE*/
/**
 * [根據指定的Key值代表特定的Message Queue和Message Type接收Request]
 * @param {key_t} key [Key值]
 * @param {REQ*} r [Request]
 * @param {long} msgtype [Message Queue Type]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int recvRequestByMSQ(key_t key, REQ *r, long msgtype) {
	int msqid;
	msqid = msgget((key_t)key, IPC_CREAT);

	MSGBUF buf;
	if(msgrcv(msqid, (void *)&buf, MSG_SIZE, msgtype, 0) >= 0) {/*return bytes sent*/
		//PrintDebug(msqid, "A request received from MSQ:");
		r->arrivalTime = buf.req.arrivalTime;
		r->devno = buf.req.devno;
		r->diskBlkno = buf.req.diskBlkno;
		r->reqSize = buf.req.reqSize;
		r->reqFlag = buf.req.reqFlag;
		r->userno = buf.req.userno;
		r->responseTime = buf.req.responseTime;
		return 0;
	}
	else 
		return -1;
}

/*TEST MESSAGE QUEUE*/
/**
 * [測試使用，僅參考，可忽略]
 */
void testMessageQueue() {
	//data structure for msg queue
	struct msqid_ds ds;
	if(createMessageQueue(KEY_MSQ_DISKSIM_1, IPC_CREAT) == -1)
		PrintError(-1, " MSQ create error in createMessageQueue():");

	REQ *r;
	r = calloc(1, sizeof(REQ));
	r->arrivalTime = 0.00;
	r->devno = 0;
	r->diskBlkno = 1234;
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

/*SEND FINISH CONTROL MESSAGE BY MESSAGE QUEUE*/
/**
 * [根據指定的Key值代表特定的Message Queue和Message Type傳送一Message(帶有特定Flag的Request)，其代表告知simulator應進行Shutdown之工作]
 * @param {key_t} key [Key值]
 * @param {long} msgtype [Message Queue Type]
 * @return {int} 0/-1 [Success(0) or Fail(-1)]
 */
int sendFinishControl(key_t key, long msgtype) {
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
