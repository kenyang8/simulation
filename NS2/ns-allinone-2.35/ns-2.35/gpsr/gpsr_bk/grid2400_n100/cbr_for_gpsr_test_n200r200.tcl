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

$ns_ at 1.5 "$ragent_(2) startSink 10"
#$ns_ at 1.6 "$ragent_(20) startSink 10"
#$ns_ at 1.7 "$ragent_(59) startSink 10"
#$ns_ at 1.8 "$ragent_(26) startSink 10"

# GPSR routing agent dumps
$ns_ at 5.0 "$ragent_(1) sinklist"




#
# nodes: 2, max conn: 5, send rate: 1.0, seed: 1
#
#
# 1 connecting to 2 at time 2.5568388786897245
#
set udp_(0) [new Agent/UDP]
$ns_ attach-agent $node_(1) $udp_(0)
set null_(0) [new Agent/Null]
$ns_ attach-agent $node_(2) $null_(0)
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ 512
$cbr_(0) set interval_ 1.0
$cbr_(0) set random_ 1
$cbr_(0) set maxpkts_ 10000
$cbr_(0) attach-agent $udp_(0)
$ns_ connect $udp_(0) $null_(0)
$ns_ at 2.5568388786897245 "$cbr_(0) start"
$ns_ at 190.54 "$cbr_(4) stop"
#
#Total sources/connections: 1/1
#


