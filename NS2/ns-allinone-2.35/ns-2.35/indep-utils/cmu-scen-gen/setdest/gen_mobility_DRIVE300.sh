MAX_NODE=200
MIN_NODE=200
seed=1
minspeed=15
maxspeed=15

node=$1
node1=201
num_sim=$2


until [  $node -gt $MAX_NODE ]
do

until [  $seed -gt $num_sim ]
do

echo "\$node_(${node}) set X_ 350.000001 
\$node_(${node}) set Y_ 350.000001
\$node_(${node}) set Z_ 0.0
\$node_(${node1}) set X_ 950.000001 
\$node_(${node1}) set Y_ 950.000001
\$node_(${node1}) set Z_ 0.0
\$ns_ at 0.0 \"\$node_(${node}) setdest 650.000001 650.000001 0\"
\$ns_ at 0.0 \"\$node_(${node1}) setdest 950.000001 950.000001 0\""   >/home/relax/Ken/simulation/VanetMobism/samples/mobility/random/r_n${node}_s${seed}.tcl


./setdest -v 2 -n ${node} -s 1 -m 14.9 -M 15 -t 50 -P 1 -p 0 -x 1200 -y 1200 >/home/relax/Ken/simulation/VanetMobism/samples/mobility/random/temp



cat /home/relax/Ken/simulation/VanetMobism/samples/mobility/random/temp >>/home/relax/Ken/simulation/VanetMobism/samples/mobility/random/r_n${node}_s${seed}.tcl
rm /home/relax/Ken/simulation/VanetMobism/samples/mobility/random/temp

let seed=seed+1
done

seed=1

let node=node+50
let node1=node1+50
done