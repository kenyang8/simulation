source "/media/simulationresult/gpsr_bk/node_start_time.tcl"
set scene_data_file "/media/simulationresult/test_e/test.tcl"

# 
# 处理节点起始和终结时间 
#
handle_node_time $scene_data_file
#
# 數據流傳輸產生 (cbrgen.tcl)
# 
# GPSR routing agent settings
for {set i 0} {$i < $opt(nn)} {incr i} {
    $ns_ at 0.00002 "$ragent_($i) turnon"
    $ns_ at 15.0 "$ragent_($i) neighborlist"

#    $ns_ at 30.0 "$ragent_($i) turnoff"
}

$ns_ at 75.0 "$ragent_(67) startSink 10"
$ns_ at 10.5 "$ragent_(50) startSink 10"
$ns_ at 10.6 "$ragent_(10) startSink 10"

# GPSR routing agent dumps
$ns_ at 25.0 "$ragent_(24) sinklist"

# create flow all you need // Mark
# Upper layer agents/applications behavior
set null_(1) [new Agent/Null]
$ns_ attach-agent $node_(10) $null_(1)

set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node_(9) $udp_(1)

set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 512
$cbr_(1) set interval_ 1.0
$cbr_(1) set random_ 1
#    $cbr_(1) set maxpkts_ 100
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 15.0 "$cbr_(1) start"
$ns_ at 99.0 "$cbr_(1) stop" 


set udp_(2) [new Agent/UDP]
$ns_ attach-agent $node_(89) $udp_(2)

set null_(2) [new Agent/Null]
$ns_ attach-agent $node_(50) $null_(2)

set cbr_(2) [new Application/Traffic/CBR]
$cbr_(2) set packetSize_ 512
$cbr_(2) set interval_ 1.0
$cbr_(2) set random_ 1
#    $cbr_(2) set maxpkts_ 2
$cbr_(2) attach-agent $udp_(2)
$ns_ connect $udp_(2) $null_(2)
$ns_ at 35.0 "$cbr_(2) start"
$ns_ at 99.0 "$cbr_(2) stop" 



set udp_(3) [new Agent/UDP]
$ns_ attach-agent $node_(51) $udp_(3)

set null_(3) [new Agent/Null]
$ns_ attach-agent $node_(67) $null_(3)

set cbr_(3) [new Application/Traffic/CBR]
$cbr_(3) set packetSize_ 512
$cbr_(3) set interval_ 1.0
$cbr_(3) set random_ 1
#    $cbr_(3) set maxpkts_ 2
$cbr_(3) attach-agent $udp_(3)
$ns_ connect $udp_(3) $null_(3)
$ns_ at 76.0 "$cbr_(3) start"
$ns_ at 99.0 "$cbr_(3) stop"
