#!/bin/bash
echo "Batch!"

ssd_parv='./../ssdmodel/valid/Intel_toolkit.parv'
hdd_parv='./../valid/maxtor146g.parv'

ssdout='./sddsim.out'
hddout='./hddsim.out'

cache='c32768_none'

#Workload
# One user
#traceU1=''
#traceU1_2=''
# Multi-user
traceU2='[u2w1w1]Financial2+WebSearch1_t2000000'
#traceU2_2=''
#traceU2_3=''
#traceU2_4=''

echo "# Only one user"
#./yusim "./../../yusim_trace/${traceU1}.yt" $ssd_parv $ssdout $hdd_parv $hddout "./output/${traceU1}_${cache}_weighted.stat" "./output/${traceU1}_${cache}_weighted.result"
#./yusim "./../../yusim_trace/${traceU1_2}.yt" $ssd_parv $ssdout $hdd_parv $hddout "./output/${traceU1_2}_${cache}_weighted.stat" "./output/${traceU1_2}_${cache}_weighted.result"

echo "# Multi-user"
./yusim "./../../yusim_trace/${traceU2}.yt" $ssd_parv $ssdout $hdd_parv $hddout "./output/${traceU2}_${cache}.stat" "./output/${traceU2}_${cache}.result"

