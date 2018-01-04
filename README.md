# Yusim

Yusim is a simple simulator based on [Disksim](http://www.pdl.cmu.edu/DiskSim/) program. We purposed to invoke two Disksim programs with two sub-processes to implement hybrid storage system.

In current version, we construct two sims(SSDsim and HDDsim) which respectively serves 'only one' request and follow a simple simulation progress(Wait/Serve/Wait/.../Shutdown). After each service, the sim returns a empty request(no other info.) with response time. That is Yusim.

Author: Yu-Cheng Yu ([g1677321@gmail.com](mailto:g1677321@gmail.com))

# Feature
## Simulator
*	Follow Disksim simulation included bus, controller and storage device.
*	Hybrid communication between SSD(considered as HDD-cache) and HDD should be concerned.
*	Replaceable caching policy.
*	Replaceable I/O request scheduler.
*	Support multi-user requests and system-defined user weights.
*	Block-level request delivery
*	Various experiment data, e.g., Response time, Throughtput, Hit rate and User info..

## Policy
*	I/O Scheduler: **Credit-based scheduler**.
*	SSD Caching/Replacement Policy: **Prize Caching**.
*	SSD Caching Space: **Logical Partition**.

# Repository contents
## /src
1.	**yu_parameter.h:** System-defined parameters. Modification should pass self-validation.
2.	**yusim.c, .h:** Main interface.
3.	**yu_syssim.c, .h:** Simulator's behaviors.
4.	**yu_ipc.c, .h:** Use message queue in "Inter-Process Communication(IPC)".
5.	**yu_structure.c, .h:** Define system structure, e.g., User queue.
6.	**yu_cache.c, .h:** SSD cacahe management.
7.	**yu_prize.c, .h:** Prize management.
8.	**yu_credit.c, .h:** Credit distribution.
9.	**yu_statistics.c, .h:** User Statistics.
10.	**yu_debug.c, .h:** -
11.	**Makefile:** Modified Disksim version.
12. **yusim_batch.sh** Batch execution.

# Trace Format
>	**_[User_Weight_1] [User_Weight_2] [User_Weight_3] ..._**<br>
>	**_[Request_Arrival_Time] [Device_number] [Block_Number] [Request_Size] [Request_Flag] [User_Number]_**<br>
>	**_[Request_Arrival_Time] [Device_number] [Block_Number] [Request_Size] [Request_Flag] [User_Number]_**<br>
>	**_[Request_Arrival_Time] [Device_number] [Block_Number] [Request_Size] [Request_Flag] [User_Number]_**<br>
>	**_..._**

1.	***[User_Weight]*** : An `unsigned integer` as "User Weight".
2.	_(Disksim-defined)_ **_[Request_Arrival_Time]_** : `Double`-precision milliseconds.
3.	_(Disksim-defined)_ **_[Device_number]_** : An `unsigned integer` as specified device. Consider multi-disk in one device parameter file(e.g., RAID).
4.	_(Disksim-defined)_ **_[Block_Number]_** : An `unsigned long` as the start of accessed block. Due to address alignment for SSD, it must be a valid number.

5.	_(Disksim-defined)_ **_[Request_Size]_** : An `unsigned integer` as the consecutive length. Due to address alignment for SSD, it must be a valid number.
6.	_(Disksim-defined)_ **_[Request_Flag]_** : **'1'** assigned as "Read" request and **'0'** assigned as "Write" request.
7.	***_[User_Number]_*** :  An `unsigned integer` user number from '1' to 'N'.

# Get started
## 1. Download Disksim4.0 & Build it!
*	[Disksim4.0](http://www.pdl.cmu.edu/DiskSim/) with [SSD patch](https://www.microsoft.com/en-us/download/details.aspx?id=52332&from=http%3A%2F%2Fresearch.microsoft.com%2Fresearch%2Fdownloads%2Fdetails%2Fb41019e2-1d2b-44d8-b512-ba35ab814cd4%2Fdetails.aspx)

## 2. Clone Yusim
## 3. Paste all files in ***/src*** into ***disksim/src***
