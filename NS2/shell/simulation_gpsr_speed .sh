

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



cat up.tcl  >mftest.tcl

echo "  

set opt(cp)            \"/home/relax/Ken/shell/cbr.tcl\"
set opt(sc)             \"/home/relax/Ken/simulation/VanetMobism/samples/mobility/n50sp${speed}s${seed}.tcl\"
set opt(nn)           50   ;# how many nodes are simulated
set opt(seed)           2.0
set opt(stop)	   350 ; "              >> mftest.tcl                                                      

cat down.tcl   >>mftest.tcl 

ns mftest.tcl 

echo  "speed = ${speed}	seed = ${seed} "          >>result_sp${speed}	
awk -f  result.awk  mflood-scene2.tr  >>result_sp${speed}	


rm mftest.tcl 


let seed=seed+1
done



grep "send: " ./result_sp${speed}  | awk '{total+=$2}END{print "send: "total}'  >temp
 grep "receive: " ./result_sp${speed}  | awk '{total+=$4}END{print "receive: "total}' >> temp
# grep "xPkts: " ./result | awk 'BEGIN{R=0;T=0}{if($2="dataRxPkts: "){R=R+$3} if($2==" dataTxPkts: "){T=T+$3}} END{print "Deliver Ratio: "T}'  > ./result_awk.tr
echo "=============================================" >>final_result_speed
echo  "speed = ${speed}	"       >>final_result_speed	
cat temp | awk '{if($1=="send:"){S=$2} if($1=="receive:"){R=$2}} END{print "Deliver Ratio: "R/S}' >> final_result_speed
rm temp


grep "r/s" ./result_sp${speed} | awk '{total+=$3; n+=1} END{print "Original r/s Ratio: "total/n}'  >> final_result_speed
 grep "delay:" ./result_sp${speed} | awk '{total+=$2; n+=1} END{print "Avg EE delay: "total/n}'  >> final_result_speed
 grep "hop count:" ./result_sp${speed} | awk '{total+=$3; n+=1} END{print "hop count: "total/n}'  >> final_result_speed
  grep "IFQ drop:" ./result_sp${speed} | awk '{total+=$3; n+=1} END{print"IFQ drop: "total/n}'  >> final_result_speed



seed=1

let speed=speed+$INTERVAL
done
