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

set opt(x)             1000   ;# X dimension of the topography
set opt(y)             1000  ;# Y dimension of the topography
set opt(cp)            "/home/relax/Ken/shell/cbr.tcl"
set opt(sc)             "/home/relax/Ken/simulation/VanetMobism/samples/mobility/n30s10.tcl"

set opt(ifqlen)         64           ;# max packet in ifq
set opt(nn)            30   ;# how many nodes are simulated
set opt(seed)           2.0
set opt(stop)	   350;
set opt(rp)   		MFlood       ;


# The transimssion radio range 
#Phy/WirelessPhy set Pt_ 8.56486e-4  ;#40m 
  
#Phy/WirelessPhy set Pt_ 7.214e-3  ;#100m 
  
#Phy/WirelessPhy set Pt_ 3.652e-2  ;#150m 
  
#Phy/WirelessPhy set Pt_ 0.1154    ;#200m 
  
#Phy/WirelessPhy set Pt_ 0.2818 ;    #250m      ||  275   2.49479e-10 || 285  2.16265e-10  || 295 1.88398e-10 || 250 3.65262e-10 || 280  2.32131e-10

 

#Phy/WirelessPhy set RXThresh_ 2.32131e-10 ;
#Phy/WirelessPhy set Pt_ 0.5843    ;#300m 
 Mac/802_11 set dataRate_ 2Mb;
 Mac/802_11 set basicRate_  1Mb;

 Phy/WirelessPhy set bandwidth_ 18Mb;

 #====================================================================== test
 # Initialize the SharedMedia interface with parameters to make
# it work like the 914MHz Lucent WaveLAN DSSS radio interface
#Phy/WirelessPhy set CPThresh_ 10.0
#Phy/WirelessPhy set CSThresh_ 1.559e-11
Phy/WirelessPhy set RXThresh_ 3.65262e-10  ;#250

#Phy/WirelessPhy set Rb_ 2*1e6
#this was the default
#Phy/WirelessPhy set Pt_ 0.2818
# This is for 100m
#Phy/WirelessPhy set Pt_ 7.214e-3
# This is for 40m
#Phy/WirelessPhy set Pt_ 8.5872e-4
# This is for 250m
#Phy/WirelessPhy set Pt_ 0.2818
#Phy/WirelessPhy set freq_ 914e+08
#Phy/WirelessPhy set L_ 1.0
# The transimssion radio range 
#Phy/WirelessPhy set Pt_ 8.56486e-4  ;#40m 
  
#Phy/WirelessPhy set Pt_ 7.214e-3  ;#100m 
  
#Phy/WirelessPhy set Pt_ 3.652e-2  ;#150m 
  
#Phy/WirelessPhy set Pt_ 0.1154    ;#200m 
  
#Phy/WirelessPhy set Pt_ 0.2818 ;    #250m 
  
#Phy/WirelessPhy set Pt_ 0.5843    ;#300m 
   
# ======================================================================
# Main Program
# ======================================================================

#ns-random 0

# Initialize Global Variables
set ns_ [new Simulator]
set tracefd [open mflood.tr w]
$ns_ trace-all $tracefd

# set up topography
set topo [new Topography]
$topo load_flatgrid $opt(x) $opt(y)

set namtrace    [open mflood.nam w]
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






