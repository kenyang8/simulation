source "/media/simulationresult/gpsr_bk/node_start_time.tcl"
set scene_data_file "/media/simulationresult/gpsr_test_n200r200/test.tcl"

# 处理节点起始和终结时间 
#
handle_node_time $scene_data_file
#
# 數據流傳輸產生 (cbrgen.tcl)
# 
# GPSR routing agent settings
for {set i 0} {$i < $opt(nn)} {incr i} {
    $ns_ at 0.00002 "$ragent_($i) turnon"
    $ns_ at 1.0 "$ragent_($i) neighborlist"

#    $ns_ at 30.0 "$ragent_($i) turnoff"
}

$ns_ at 1.5 "$ragent_(4) startSink 10"
$ns_ at 1.6 "$ragent_(20) startSink 10"
$ns_ at 1.7 "$ragent_(59) startSink 10"
$ns_ at 1.8 "$ragent_(26) startSink 10"

# GPSR routing agent dumps
$ns_ at 5.0 "$ragent_(24) sinklist"


#
# 4 connecting to 5 at time 56.333118917575632
#
set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node_(6) $udp_(1)
set null_(1) [new Agent/Null]
$ns_ attach-agent $node_(4) $null_(1)
set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 1000
$cbr_(1) set interval_ 1.0
$cbr_(1) set random_ 1
$cbr_(1) set maxpkts_ 10000
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 10.333 "$cbr_(1) start"
$ns_ at 190.333 "$cbr_(1) stop"

#
# 4 connecting to 6 at time 146.96568928983328
#
set udp_(2) [new Agent/UDP]
$ns_ attach-agent $node_(24) $udp_(2)
set null_(2) [new Agent/Null]
$ns_ attach-agent $node_(20) $null_(2)
set cbr_(2) [new Application/Traffic/CBR]
$cbr_(2) set packetSize_ 1000
$cbr_(2) set interval_ 1.0
$cbr_(2) set random_ 1
$cbr_(2) set maxpkts_ 10000
$cbr_(2) attach-agent $udp_(2)
$ns_ connect $udp_(2) $null_(2)
$ns_ at 25.965 "$cbr_(2) start"
$ns_ at 190.965 "$cbr_(2) stop"
#
# 6 connecting to 7 at time 55.634230382570173
#
set udp_(3) [new Agent/UDP]
$ns_ attach-agent $node_(81) $udp_(3)
set null_(3) [new Agent/Null]
$ns_ attach-agent $node_(59) $null_(3)
set cbr_(3) [new Application/Traffic/CBR]
$cbr_(3) set packetSize_ 1000
$cbr_(3) set interval_ 1.0
$cbr_(3) set random_ 1
$cbr_(3) set maxpkts_ 10000
$cbr_(3) attach-agent $udp_(3)
$ns_ connect $udp_(3) $null_(3)
$ns_ at 140.634 "$cbr_(3) start"
$ns_ at 190.634 "$cbr_(3) stop"
#
# 7 connecting to 8 at time 29.546173154165118
#
set udp_(4) [new Agent/UDP]
$ns_ attach-agent $node_(129) $udp_(4)
set null_(4) [new Agent/Null]
$ns_ attach-agent $node_(26) $null_(4)
set cbr_(4) [new Application/Traffic/CBR]
$cbr_(4) set packetSize_ 1000
$cbr_(4) set interval_ 1.0
$cbr_(4) set random_ 1
$cbr_(4) set maxpkts_ 10000
$cbr_(4) attach-agent $udp_(4)
$ns_ connect $udp_(4) $null_(4)
$ns_ at 60.54 "$cbr_(4) start"
$ns_ at 190.54 "$cbr_(4) stop"
