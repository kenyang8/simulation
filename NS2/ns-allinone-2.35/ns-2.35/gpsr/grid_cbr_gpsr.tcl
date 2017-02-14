
source "./node_start_time.tcl"
set scene_data_file "./grid_scense.tcl"

# 
handle_node_time $scene_data_file

set after_turnon 10.0  
# GPSR routing agent settings
set move_node_num [expr $opt(nn)-1]
for {set i 0} {$i < $move_node_num } {incr i} {
   # puts $arr_node($i)
    $ns_ at $arr_node($i) "$ragent_($i) turnon"
    $ns_ at [expr $arr_node($i)+$after_turnon]  "$ragent_($i) startSink 10.0 "
  #  $ns_ at [expr $arr_node($i)+1]  "$ragent_($i) neighborlist"
    $ns_ at $arr_node_end($i) "$ragent_($i) turnoff"
}

set cbr_start_time 54
set n 0
set end_base_station $move_node_num

for {set i 0 } { $i < $move_node_num } {incr i} {
    if  { [expr $arr_node($i)-$cbr_start_time  ] > 0 } then {

	if { [expr $i % 2] == 1 } then {
	    set rec_node  [expr $i - 2 ]
	} else {
	    set rec_node  $end_base_station
	}

	set send_node $i
	set null_($n) [new Agent/Null]
	$ns_ attach-agent $node_($rec_node) $null_($n)

	set udp_($n) [new Agent/UDP]
	$ns_ attach-agent $node_($send_node) $udp_($n)

	set cbr_($n) [new Application/Traffic/CBR]
	$cbr_($n) set packetSize_ 32
	$cbr_($n) set interval_ 2.0
	$cbr_($n) set random_ 1
	#    $cbr_(1) set maxpkts_ 100
	$cbr_($n) attach-agent $udp_($n)
	$ns_ connect $udp_($n) $null_($n)

	$ns_ at [expr $arr_node($i)+$after_turnon+1]   "$cbr_($n) start"
	$ns_ at $arr_node_end([expr $i-2])  "$cbr_($n) stop" 
	incr n 

    }

}