source "/home/relax/Ken/simulation/node_start_time.tcl"
set scene_data_file "/home/relax/Ken/simulation/T40/T200.tcl"

# 处理节点起始和终结时间 
#
handle_node_time $scene_data_file
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
$ns_ attach-agent $node_(0) $udp_(0)
set null_(0) [new Agent/Null]
$ns_ attach-agent $node_(10) $null_(0)
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ 512
$cbr_(0) set interval_ 1.0
#$cbr_(0) set rate_ 2Mb                                 
$cbr_(0) set random_ 1
$cbr_(0) set maxpkts_ 100000
$cbr_(0) attach-agent $udp_(0)
$ns_ connect $udp_(0) $null_(0)
$ns_ at 31.5568388786897245 "$cbr_(0) start"
$ns_ at 231.5568388  "$cbr_(0) stop"
#
#Total sources/connections: 1/1
#0.01041666


set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node_(1) $udp_(1)
set null_(1) [new Agent/Null]
$ns_ attach-agent $node_(11) $null_(1)
set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 512
$cbr_(1) set interval_ 1.0
#$cbr_(1) set rate_ 2Mb                                 
$cbr_(1) set random_ 1
$cbr_(1) set maxpkts_ 100000
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 60.5568388786897245 "$cbr_(1) start"
$ns_ at 260.5568388  "$cbr_(1) stop"
#=========================================================
set udp_(2) [new Agent/UDP]
$ns_ attach-agent $node_(2) $udp_(2)
set null_(2) [new Agent/Null]
$ns_ attach-agent $node_(12) $null_(2)
set cbr_(2) [new Application/Traffic/CBR]
$cbr_(2) set packetSize_ 512
$cbr_(2) set interval_ 1.0
#$cbr_(2) set rate_ 2Mb                                 
$cbr_(2) set random_ 1
$cbr_(2) set maxpkts_ 100000
$cbr_(2) attach-agent $udp_(2)
$ns_ connect $udp_(2) $null_(2)
$ns_ at 90.5568388786897245 "$cbr_(2) start"
$ns_ at 290.5568388  "$cbr_(2) stop"
#=========================================================
set udp_(3) [new Agent/UDP]
$ns_ attach-agent $node_(3) $udp_(3)
set null_(3) [new Agent/Null]
$ns_ attach-agent $node_(13) $null_(3)
set cbr_(3) [new Application/Traffic/CBR]
$cbr_(3) set packetSize_ 512
$cbr_(3) set interval_ 1.0
#$cbr_(2) set rate_ 2Mb                                 
$cbr_(3) set random_ 1
$cbr_(3) set maxpkts_ 100000
$cbr_(3) attach-agent $udp_(3)
$ns_ connect $udp_(3) $null_(3)
$ns_ at 120.5568388786897245 "$cbr_(3) start"
$ns_ at 320.5568388  "$cbr_(3) stop"