#Agent/UDP set packetSize_ 6000

# ======================================================================
# Define options
# ======================================================================
set opt(chan)       Channel/WirelessChannel
set opt(prop)       Propagation/TwoRayGround
set opt(netif)      Phy/WirelessPhy
set opt(mac)        Mac/802_11
set opt(ifq)        Queue/DropTail/PriQueue
set opt(ll)         LL
set opt(ant)        Antenna/OmniAntenna

set opt(x)            952   ;# X dimension of the topography
set opt(y)           952 ;# Y dimension of the topography
set opt(cp)            "/home/relax/Ken/simulation/T40/cbr40.tcl"
set opt(sc)             "/home/relax/Ken/simulation/sp15-n/n50.tcl"

set opt(ifqlen)         60000            ;# max packet in ifq
set opt(nn)          51;# how many nodes are simulated
set opt(seed)          0.0
set opt(stop)	  245.0 ;
set opt(rp)   		MFlood       ;

LL set mindelay_		50us
LL set delay_			25us
LL set bandwidth_		0	;# not used

Agent/Null set sport_		0
Agent/Null set dport_		0

Agent/CBR set sport_		0
Agent/CBR set dport_		0

Agent/TCPSink set sport_	0
Agent/TCPSink set dport_	0

Agent/TCP set sport_		0
Agent/TCP set dport_		0
Agent/TCP set packetSize_	1460

Queue/DropTail/PriQueue set Prefer_Routing_Protocols    1

# unity gain, omni-directional antennas
# set up the antennas to be centered in the node and 1.5 meters above it
Antenna/OmniAntenna set X_ 0
Antenna/OmniAntenna set Y_ 0
Antenna/OmniAntenna set Z_ 1.5
Antenna/OmniAntenna set Gt_ 1.0
Antenna/OmniAntenna set Gr_ 1.0





# ======================================================================

# Agent/GPSR setting
Agent/GPSR set planar_type_  1   ;#1=GG planarize, 0=RNG planarize
Agent/GPSR set hello_period_ 5.0 ;#Hello message period
Agent/GPSR set carry_period_ 5.0 ;#carry and forward period
Agent/GPSR set update_sink_loc_period_ 0.500
Agent/GPSR set start_update_time_ 0.002
Agent/GPSR set safe_distance_ 245
Agent/GPSR set delperiod 3
Agent/GPSR set usefulperiod 3
# ======================================================================
# The transimssion radio range 
#Phy/WirelessPhy set Pt_ 8.56486e-4  ;#40m 
  
#Phy/WirelessPhy set Pt_ 7.214e-3  ;#100m 
  
#Phy/WirelessPhy set Pt_ 3.652e-2  ;#150m 
  
#Phy/WirelessPhy set Pt_ 0.1154    ;#200m 
  
Phy/WirelessPhy set Pt_ 0.2818 ;    #250m 
  
#Phy/WirelessPhy set Pt_ 0.5843    ;#300m 
# Mac/802_11 set dataRate_ 2Mb;
# Mac/802_11 set basicRate_  1Mb;

 Phy/WirelessPhy set bandwidth_ 6Mb;
 # Initialize the SharedMedia interface with parameters to make
# it work like the 914MHz Lucent WaveLAN DSSS radio interface
Phy/WirelessPhy set CPThresh_ 10.0
Phy/WirelessPhy set CSThresh_ 1.559e-11
Phy/WirelessPhy set RXThresh_ 3.652e-10
Phy/WirelessPhy set Rb_ 2*1e6
Phy/WirelessPhy set freq_ 914e+6 
Phy/WirelessPhy set L_ 1.0  
# ======================================================================
# Main Program
# ======================================================================

#ns-random 0

# Initialize Global Variables
set ns_ [new Simulator]
set tracefd [open mflood-scene2.tr w]
$ns_ trace-all $tracefd

# set up topography
set topo [new Topography]
$topo load_flatgrid $opt(x) $opt(y)

set namtrace    [open wireless1-out.nam w]
$ns_ namtrace-all-wireless $namtrace $opt(x) $opt(y)

#
# Create God
#
set god_ [create-god $opt(nn)]

# Create the specified number of mobilenodes [$opt(nn)] and "attach" them
# to the channel. 
# configure node
set channel [new Channel/WirelessChannel]
$channel set errorProbability_ 0.0

        $ns_ node-config -adhocRouting $opt(rp) \
			 -llType $opt(ll) \
			 -macType $opt(mac) \
			 -ifqType $opt(ifq) \
			 -ifqLen $opt(ifqlen) \
			 -antType $opt(ant) \
			 -propType $opt(prop) \
			 -phyType $opt(netif) \
			 -channel $channel \
			 -topoInstance $topo \
			 -agentTrace ON \
			 -routerTrace ON\
			 -macTrace OFF \
			 -movementTrace OFF			
			 
	for {set i 0} {$i < $opt(nn) } {incr i} {
		set node_($i) [$ns_ node]	
		$node_($i) random-motion 0;	
	}

#
# Define node movement model
#
puts "Loading connection pattern..."
source $opt(cp)

#
# Define traffic model
#
puts "Loading scenario file..."
source $opt(sc)


# Define node initial position in nam

#for {set i 0} {$i < $opt(nn)} {incr i} {

    # 20 defines the node size in nam, must adjust it according to your scenario
    # The function must be called after mobility model is defined

 #   $ns_ initial_node_pos $node_($i) 20
#}

# Tell nodes when the simulation ends
#for {set i 0} {$i < $opt(nn) } {incr i} {
#    $ns_ at $opt(stop).0 "$node_($i) reset";
#}

$ns_ at $opt(stop).0 "stop"
$ns_ at $opt(stop).01 "puts \"NS EXITING...\" ; $ns_ halt"

proc stop {} {
    global ns_ tracefd
    $ns_ flush-trace
    close $tracefd
}

puts "Starting Simulation..."
$ns_ run






