source "/media/simulationresult/gpsr_bk/node_start_time.tcl"
set scene_data_file "/media/simulationresult/gpsr_test_n600r600/test.tcl"

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

$ns_ at 1.5 "$ragent_(23) startSink 10"
$ns_ at 1.6 "$ragent_(48) startSink 10"
$ns_ at 1.7 "$ragent_(114) startSink 10"
$ns_ at 1.8 "$ragent_(38) startSink 10"
$ns_ at 1.9 "$ragent_(89) startSink 10"
$ns_ at 2.0 "$ragent_(210) startSink 10"
# GPSR routing agent dumps
$ns_ at 5.0 "$ragent_(24) sinklist"


#
# 4 connecting to 5 at time 56.333118917575632
#
set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node_(26) $udp_(1)
set null_(1) [new Agent/Null]
$ns_ attach-agent $node_(23) $null_(1)
set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 1000
$cbr_(1) set interval_ 1.0
$cbr_(1) set random_ 1
$cbr_(1) set maxpkts_ 10000
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 30.333 "$cbr_(1) start"
#$ns_ at 60.333 "$cbr_(1) stop"

#
# 4 connecting to 6 at time 146.96568928983328
#
set udp_(2) [new Agent/UDP]
$ns_ attach-agent $node_(30) $udp_(2)
set null_(2) [new Agent/Null]
$ns_ attach-agent $node_(48) $null_(2)
set cbr_(2) [new Application/Traffic/CBR]
$cbr_(2) set packetSize_ 1000
$cbr_(2) set interval_ 1.0
$cbr_(2) set random_ 1
$cbr_(2) set maxpkts_ 10000
$cbr_(2) attach-agent $udp_(2)
$ns_ connect $udp_(2) $null_(2)
$ns_ at 81.965 "$cbr_(2) start"
#$ns_ at 110.965 "$cbr_(2) stop"
#
# 6 connecting to 7 at time 55.634230382570173
#
set udp_(3) [new Agent/UDP]
$ns_ attach-agent $node_(13) $udp_(3)
set null_(3) [new Agent/Null]
$ns_ attach-agent $node_(114) $null_(3)
set cbr_(3) [new Application/Traffic/CBR]
$cbr_(3) set packetSize_ 1000
$cbr_(3) set interval_ 1.0
$cbr_(3) set random_ 1
$cbr_(3) set maxpkts_ 10000
$cbr_(3) attach-agent $udp_(3)
$ns_ connect $udp_(3) $null_(3)
$ns_ at 122.634 "$cbr_(3) start"
#$ns_ at 150.634 "$cbr_(3) stop"
#
# 7 connecting to 8 at time 29.546173154165118
#
set udp_(4) [new Agent/UDP]
$ns_ attach-agent $node_(118) $udp_(4)
set null_(4) [new Agent/Null]
$ns_ attach-agent $node_(38) $null_(4)
set cbr_(4) [new Application/Traffic/CBR]
$cbr_(4) set packetSize_ 1000
$cbr_(4) set interval_ 1.0
$cbr_(4) set random_ 1
$cbr_(4) set maxpkts_ 10000
$cbr_(4) attach-agent $udp_(4)
$ns_ connect $udp_(4) $null_(4)
$ns_ at 150.54 "$cbr_(4) start"
#$ns_ at 180.54 "$cbr_(4) stop"

set udp_(5) [new Agent/UDP]
$ns_ attach-agent $node_(160) $udp_(5)
set null_(5) [new Agent/Null]
$ns_ attach-agent $node_(89) $null_(5)
set cbr_(5) [new Application/Traffic/CBR]
$cbr_(5) set packetSize_ 1000
$cbr_(5) set interval_ 1.0
$cbr_(5) set random_ 1
$cbr_(5) set maxpkts_ 10000
$cbr_(5) attach-agent $udp_(5)
$ns_ connect $udp_(5) $null_(5)
$ns_ at 193.54 "$cbr_(5) start"
#$ns_ at 220.54 "$cbr_(5) stop"

set udp_(6) [new Agent/UDP]
$ns_ attach-agent $node_(206) $udp_(6)
set null_(6) [new Agent/Null]
$ns_ attach-agent $node_(210) $null_(6)
set cbr_(6) [new Application/Traffic/CBR]
$cbr_(6) set packetSize_ 1000
$cbr_(6) set interval_ 1.0
$cbr_(6) set random_ 1
$cbr_(6) set maxpkts_ 10000
$cbr_(6) attach-agent $udp_(6)
$ns_ connect $udp_(6) $null_(6)
$ns_ at 238.54 "$cbr_(6) start"
#$ns_ at 270.54 "$cbr_(6) stop"
