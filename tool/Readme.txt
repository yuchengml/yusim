first lime in trace:
[user_1_weight] [user_2_weight] ... [user_N_weight]

[Request arrival time] [Device number] [Block number] [Request size] [Request flags(read:1;write:0)] [User number(1-N)]


1.
	[u1]代表單一使用者的Workload；[u2w1w1]代表兩個使用者的Workload，且權重為1：1
2.
	Workload名稱_t擷取時間
3.
	通常，單一使用者的Workload的Blkno會從0開始編號，時間也從0開始
	而兩個使用者混和的Workload，使用者2會將blkno推移至使用者1之後，時間也從0開始，並將兩者混和

P.S.

Tool:
	(a) yu_trace:		觀察Trace file資訊，根據程式內Function使用
	(b) insertUserNum: 	新增使用者編號（提醒Yusim起始編號為1，與disksim相同）（預設前五個欄位為已知）
	(c) numbering: 		將blkno重新編號(可定義起始編號)
	(d) synthesizer:	將Workload合成（根據時間排序）（請注意！此程式不會驗證Input file的時間是否循序正確）