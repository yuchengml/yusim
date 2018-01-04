# Tools
1.	**yu_trace:** 觀察Trace file資訊，根據程式內Function的使用需求
2.	**insertUserNum:** 新增使用者編號（提醒Yusim起始編號為1，與disksim相同）（預設前五個欄位為已知）
3.	**numbering:** 將blkno重新編號(可定義起始編號)
4.	**synthesizer:** 將Workload合成（根據時間排序）（請注意！此程式不會驗證Input file的時間是否循序正確）

# Trace Format
>	**_[User_Weight_1] [User_Weight_2] [User_Weight_3] ... [User_Weight_N]_**<br>
>	**_[Request_Arrival_Time] [Device_number] [Block_Number] [Request_Size] [Request_Flag(read:1;write:0)] [User_Number(1)]_**<br>
>	**_[Request_Arrival_Time] [Device_number] [Block_Number] [Request_Size] [Request_Flag(read:1;write:0)] [User_Number(2)]_**<br>
>	**_[Request_Arrival_Time] [Device_number] [Block_Number] [Request_Size] [Request_Flag(read:1;write:0)] [User_Number(3)]_**<br>
>	**_..._**<br>
>	**_[Request_Arrival_Time] [Device_number] [Block_Number] [Request_Size] [Request_Flag(read:1;write:0)] [User_Number(N)]_**<br>

# Rules
1. [u1]代表單一使用者的Workload；[u2w1w1]代表兩個使用者的Workload，且權重為1：1
2. Workload名稱_t擷取時間
3. 實驗中，單一使用者的Workload的Block number通常從0開始編號，時間也從0開始；而多使用者混和的Workload，假設未包含相同資料的情況下，相異使用者具有相異的Block number。舉例而言，將使用者2的最小Block number推移至使用者1之後。時間則必須保持循序性進行
