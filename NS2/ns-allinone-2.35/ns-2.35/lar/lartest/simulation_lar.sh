grep "dataRxPkts: " ./result | awk '{total+=$2}END{print "RX "total}'  > temp
 grep "dataTxPkts: " ./result | awk '{total+=$2}END{print "TX "total}' >> temp
# grep "xPkts: " ./result | awk 'BEGIN{R=0;T=0}{if($2="dataRxPkts: "){R=R+$3} if($2==" dataTxPkts: "){T=T+$3}} END{print "Deliver Ratio: "T}'  > ./result_awk.tr
cat temp | awk '{if($1=="RX"){R=$2} if($1=="TX"){T=$2}} END{print "Deliver Ratio: "R/T}' > ./result_awk.tr
rm temp



 grep "Average EE" ./result | awk '{total+=$4; n+=1} END{print "Avg EE delay: "total/n}'  >> ./result_awk.tr
 grep "Average Hops" ./result | awk '{total+=$3; n+=1} END{print "Avg Hops: "total/n}'  >> ./result_awk.tr
grep "dataRxPkts: " ./result   >> ./result_awk.tr
 grep "dataTxPkts: " ./result  >> ./result_awk.tr