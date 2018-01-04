#ifndef YU_PARAMETER_H
#define YU_PARAMETER_H
	
	/*Options*/
	/*<A> YUSIM_CREDIT_BASED_SCHEDULER
	 *<B> YUSIM_CREDIT_BASED_SCHEDULER_WORK_CONSERVING
	 *<C> YUSIM_LOGICAL_PARTITION
	 */
	/*-------------------------------------------------------------*/
	/*Modes*/
	//Scheduler, Work-Conserving(WC or NoWC), Caching Algo. and Logical Cache Partition
	//Prize Caching is a default caching algorithm. 
	/*(1) FCFS:            <B>
	 *(2) FCFS + LCP:      <B>+<C>
	 *(3) MTS(NoWC):       <A>
	 *(4) MTS(WC):         <A>+<B>
	 *(5) MTS(NoWC) + LCP: <A>+<C>
	 *(6) MTS(WC) + LCP:   <A>+<B>+<C>
	 */

	//Please comment the following definition and RE-MAKE yusim!
	#define YUSIM_CREDIT_BASED_SCHEDULER
	#define YUSIM_CREDIT_BASED_SCHEDULER_WORK_CONSERVING
	#define YUSIM_LOGICAL_PARTITION
	/*-------------------------------------------------------------*/

	/*SIMULATOR*/
	#define DISKSIM_SECTOR   512  //(bytes)
	#define SSD_PAGE_SIZE    4096 //(bytes)
	#define SSD_PAGE2SECTOR (SSD_PAGE_SIZE/DISKSIM_SECTOR)
	#define SSD_PAGES_PER_BLOCK 64
	#define SSD_BLOCK_SIZE   (SSD_PAGE_SIZE*SSD_PAGES_PER_BLOCK) //(bytes)
	//#define SSD_BLOCK2SECTOR (SSD_BLOCK_SIZE/DISKSIM_SECTOR)

	#define TIME_PERIOD 1000 //ms  //VSSD uses 1000.0
	#define STAT_FOR_TIME_PERIODS 10 // Once writeStatFile() for ? TIME_PERIODS
	#define NUM_OF_USER 2
	//Hint: user number in trace file is form 1 to n
	
	#define SSD_CACHING_SPACE_BY_PAGES 32768
	//MAX:(8*8*2048*64*8(channels) = 67108864 sectors)(67108864/PAGE2SECTOR = 8388608)
	//Hint: < 6291456 page valid! 
	
	#define SSD_N_ELEMENTS 1 //SSD Channels //No multi channel

	/*ipc*/
	//One message is considered as one request. The control message is a flag which used to control simulator
	#define KEY_MSQ_DISKSIM_1 0x0015		//The key of message queue for SSD simulator
	#define KEY_MSQ_DISKSIM_2 0x0026		//The key of message queue for HDD simulator
	#define MSG_TYPE_DISKSIM_1 100			//The type of message for SSD simulator
	#define MSG_TYPE_DISKSIM_1_SERVED 101	//The type of served message for SSD simulator
	#define MSG_TYPE_DISKSIM_2 200			//The type of message for HDD simulator
	#define MSG_TYPE_DISKSIM_2_SERVED 201	//The type of served message for HDD simulator
	#define MSG_REQUEST_CONTROL_FLAG_FINISH 999		//The type of control message for simulator

	/*cache*/
	#define PAGE_FLAG_FREE 0		//The flag of free page in cache table 
	#define PAGE_FLAG_NOT_FREE 1	//The flag of non-free page in cache table 
	#define PAGE_FLAG_CLEAN 1		//The flag of clean page in cache table 
	#define PAGE_FLAG_DIRTY -1		//The flag of dirty page in cache table 
	#define CACHE_FULL 1		//The flag of cache means that cache is full
	#define CACHE_NOT_FULL 0	//The flag of cache means that cache is not full

	/*prize caching*/
	#define MIN_PRIZE 0.0		//The minimal prize in prize caching. It's a threshold which excludes data from cache
	#define ALPHA 0.5			//The percentage of recency in the core function of prize caching
	
	/*credit*/
	#define INIT_CREDIT (TIME_PERIOD*SSD_N_ELEMENTS)		//The initial credit

	/*Others*/
	#define PAUSE //getchar();
	//ASCII Escape Code
	#define COLOR_RB "\x1b[;31;1m"
	#define COLOR_GB "\x1b[;32;1m"
	#define COLOR_YB "\x1b[;33;1m"
	#define COLOR_BB "\x1b[;34;1m"
	#define COLOR_N "\x1b[0;m"

#endif
