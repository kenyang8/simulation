

MAX_SPEED=100
MIN_SPEED=0
INTERVAL=20

speed=$MIN_SPEED
simulations_time=350
seed=1
num_sim=50



until [  $speed -gt $MAX_SPEED ]
do

until [  $seed -gt $num_sim ]
do

#home/relax/Ken/simulation/VanetMobism/samples/gen_mobility.sh   $node $simulations_time $seed $num_sim





echo "  

set val(chan)       Channel/WirelessChannel
set val(prop)       Propagation/TwoRayGround
set val(netif)      Phy/WirelessPhy
set val(mac)        Mac/802_11
#set val(ifq)        Queue/DropTail/PriQueue
set val(ll)         LL
set val(ant)        Antenna/OmniAntenna
set val(x)          1000
set val(y)          1000
set val(ifqlen)     64
set val(rp)         AODV
#set val(rp)          DSR
#set val(rp)         DSDV
set val(nn)         50
#set val(txPower)    0.0075  ; #100 meters   # 傳送消耗的電力
#set val(rxPower)    1          ; # 接收消耗的電力
set val(sc)          \"/home/relax/Ken/simulation/lar2009_mobility_set/n50sp${speed}s${seed}\"

set val(dataStart)  25.0
set val(dataStop)   275.0
set val(signalStop) 305.0
set val(finish)     310.0
" > lar.tcl                  

cat down_lar_b.tcl   >>lar.tcl 

ns lar.tcl  >result
echo "=============================================" >>result_sp${speed}
echo  "speed = ${speed}	seed = ${seed} "          >>result_sp${speed}

grep "dataRxPkts: " ./result | awk '{total+=$2}END{print "RX "total}'  > temp
 grep "dataTxPkts: " ./result | awk '{total+=$2}END{print "TX "total}' >> temp
# grep "xPkts: " ./result | awk 'BEGIN{R=0;T=0}{if($2="dataRxPkts: "){R=R+$3} if($2==" dataTxPkts: "){T=T+$3}} END{print "Deliver Ratio: "T}'  > ./result_awk.tr
cat temp | awk '{if($1=="RX"){R=$2} if($1=="TX"){T=$2}} END{print "Deliver Ratio: "R/T}' >>result_sp${speed}
rm temp



 grep "Average EE" ./result | awk '{total+=$4; n+=1} END{print "Avg EE delay: "total/n}'  >>result_sp${speed}
 grep "Average Hops" ./result | awk '{total+=$3; n+=1} END{print "Avg Hops: "total/n}'  >>result_sp${speed}
#grep "dataRxPkts: " ./result   >>result_sp${speed}
 #grep "dataTxPkts: " ./result  >>result_sp${speed}


rm lar.tcl


let seed=seed+1
done




echo "=============================================" >>final_result
echo  "speed = ${speed}	"       >>final_result	

 grep "Deliver Ratio" ./result_sp${speed}| awk '{total+=$3; n+=1} END{print "Deliver Ratio: "total/n}'  >>final_result	
 grep "Avg EE" ./result_sp${speed}| awk '{total+=$4; n+=1} END{print "Avg EE delay: "total/n}'  >>final_result	
 grep "Avg Hops" ./result_sp${speed}| awk '{total+=$3; n+=1} END{print "Avg Hops: "total/n}' >>final_result	
#grep "dataRxPkts: " ./result_sp${speed}  >>final_result	
 #grep "dataTxPkts: " ./result_sp${speed}  >>final_result	


seed=1




let speed=speed+$INTERVAL
done




