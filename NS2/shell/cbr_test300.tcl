#source "/home/relax/Ken/simulation/node_start_time.tcl"
#set scene_data_file "/home/relax/Ken/simulation/T40/T200.tcl"

# 处理节点起始和终结时间 
#
#handle_node_time $scene_data_file
#
# 數據流傳輸產生 (cbrgen.tcl)
# 
# GPSR routing agent settings kkkk




#
# nodes: 2, max conn: 5, send rate: 1.0, seed: 1
#
#
# 1 connecting to 2 at time 2.556838c
#
set udp_(0) [new Agent/UDP]
$ns_ attach-agent $node_(300) $udp_(0)
set null_(0) [new Agent/Null]
$ns_ attach-agent $node_(301) $null_(0)
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ 1000
$cbr_(0) set interval_ 1.0
#$cbr_(0) set rate_ 0.2Mb                                 
$cbr_(0) set random_ 1
$cbr_(0) set maxpkts_ 100000
$cbr_(0) attach-agent $udp_(0)
$ns_ connect $udp_(0) $null_(0)
$ns_ at 20.5568388786897248 "$cbr_(0) start"
$ns_ at 21.0068388  "$cbr_(0) stop"
#
#Total sources/connections: 1/1
#0.01041666


