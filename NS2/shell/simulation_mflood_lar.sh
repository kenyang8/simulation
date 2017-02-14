

MAX_NODE=70
MIN_NODE=30
INTERVAL=10

node=$MIN_NODE
simulations_time=400
seed=1
num_sim=10


until [  $node -gt $MAX_NODE ]
do

until [  $seed -gt $num_sim ]
do


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
set val(nn)         ${node}
#set val(txPower)    0.0075  ; #100 meters   # 傳送消耗的電力
#set val(rxPower)    1          ; # 接收消耗的電力
set val(sc)             \"/home/relax/Ken/simulation/VanetMobism/samples/mobility/n${node}s${seed}.tcl\"


set val(dataStart)  30.0
set val(dataStop)   230.0
set val(signalStop) 250.0
set val(finish)     250.0
" > lar.tcl                  

cat down_lar_b.tcl   >>lar.tcl 

ns lar.tcl  >result
echo "=============================================" >>result_n${node}
echo  "noede = ${node}		seed = ${seed} "          >>result_n${node}

grep "dataRxPkts: " ./result | awk '{total+=$2}END{print "RX "total}'  > temp
 grep "dataTxPkts: " ./result | awk '{total+=$2}END{print "TX "total}' >> temp
# grep "xPkts: " ./result | awk 'BEGIN{R=0;T=0}{if($2="dataRxPkts: "){R=R+$3} if($2==" dataTxPkts: "){T=T+$3}} END{print "Deliver Ratio: "T}'  > ./result_awk.tr
cat temp | awk '{if($1=="RX"){R=$2} if($1=="TX"){T=$2}} END{print "Deliver Ratio: "R/T}' >>result_n${node}
rm temp



 grep "Average EE" ./result | awk '{total+=$4; n+=1} END{print "Avg EE delay: "total/n}'  >>result_n${node}
 grep "Average Hops" ./result | awk '{total+=$3; n+=1} END{print "Avg Hops: "total/n}'  >>result_n${node}
#grep "dataRxPkts: " ./result   >>result_n${node}
 #grep "dataTxPkts: " ./result  >>result_n${node}


rm lar.tcl


let seed=seed+1
done




echo "=============================================" >>final_result
echo  "node= ${node}	"       >>final_result	

 grep "Deliver Ratio" ./result_n${node}| awk '{total+=$3; n+=1} END{print "Deliver Ratio: "total/n}'  >>final_result	
 grep "Avg EE" ./result_n${node}| awk '{total+=$4; n+=1} END{print "Avg EE delay: "total/n}'  >>final_result	
 grep "Avg Hops" ./result_n${node}| awk '{total+=$3; n+=1} END{print "Avg Hops: "total/n}' >>final_result	
#grep "dataRxPkts: " ./result_n${node}  >>final_result	
 #grep "dataTxPkts: " ./result_n${node}  >>final_result	


seed=1




let node=node+$INTERVAL
done