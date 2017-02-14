

MAX_SPEED=25
MIN_SPEED=5
INTERVAL=5

speed=$MIN_SPEED
simulations_time=400
seed=1
num_sim=10


until [  $speed -gt $MAX_SPEED ]
do

until [  $seed -gt $num_sim ]
do

#/home/relax/Ken/simulation/VanetMobism/samples/gen_mobility_speed.sh   $speed $simulations_time $seed $num_sim




echo "  
set opt(chan)		Channel/WirelessChannel
set opt(prop)		Propagation/TwoRayGround
set opt(netif)		Phy/WirelessPhy
set opt(mac)		Mac/802_11
set opt(ifq)		Queue/DropTail/PriQueue	;# for dsdv
set opt(ll)			LL
set opt(ant)        Antenna/OmniAntenna

set opt(x)		1000; # X dimension of the topography 952 300
set opt(y)		1000; #Y dimension of the topography  952 300
set opt(cp)		\"/home/relax/Ken/shell/gcbr.tcl\"
set opt(sc)		\"/home/relax/Ken/simulation/VanetMobism/samples/mobility/n50sp${speed}s${seed}.tcl\"

set opt(ifqlen)		9000	;# max packet in ifq
set opt(nn)			${node}	;# number of nodes
set opt(seed)		${seed}
set opt(stop)		350		;# simulation time
set opt(tr)			run_trace.tr	;# trace file
set opt(nam)        run_out.nam
set opt(rp)         gpsr		;# routing protocol script (dsr or dsdv)
set opt(lm)        	\"off\"		;# log movement(default: off)         " > gpsr.tcl                  

cat down_gpsr.tcl   >>gpsr.tcl 
                              



ns gpsr.tcl 

echo  "speed = ${speed}	seed = ${seed} "          >>result_sp${speed}_gpsr	
awk -f  result.awk  run_trace.tr  >>result_sp${speed}_gpsr	


rm gpsr.tcl 


let seed=seed+1
done



grep "send: " ./result_sp${speed}_gpsr  | awk '{total+=$2}END{print "send: "total}'  >temp
 grep "receive: " ./result_sp${speed}_gpsr  | awk '{total+=$4}END{print "receive: "total}' >> temp
# grep "xPkts: " ./result | awk 'BEGIN{R=0;T=0}{if($2="dataRxPkts: "){R=R+$3} if($2==" dataTxPkts: "){T=T+$3}} END{print "Deliver Ratio: "T}'  > ./result_awk.tr
echo "=============================================" >>final_result_speed
echo  "speed = ${speed}	"       >>final_result_speed	
cat temp | awk '{if($1=="send:"){S=$2} if($1=="receive:"){R=$2}} END{print "Deliver Ratio: "R/S}' >> final_result_speed
rm temp


grep "r/s" ./result_sp${speed}_gpsr | awk '{total+=$3; n+=1} END{print "Original r/s Ratio: "total/n}'  >> final_result_speed
 grep "delay:" ./result_sp${speed}_gpsr | awk '{total+=$2; n+=1} END{print "Avg EE delay: "total/n}'  >> final_result_speed
 grep "hop count:" ./result_sp${speed}_gpsr | awk '{total+=$3; n+=1} END{print "hop count: "total/n}'  >> final_result_speed
  grep "IFQ drop:" ./result_sp${speed}_gpsr | awk '{total+=$3; n+=1} END{print"IFQ drop: "total/n}'  >> final_result_speed



seed=1

let speed=speed+$INTERVAL
done
