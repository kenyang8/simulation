

MAX_SPEED=20
MIN_SPEED=0
INTERVAL=5
INTERVAL_1=1
INTERVAL_4=4

speed=$MIN_SPEED
simulations_time=2400
seed=1
num_sim=1


until [  $speed -gt $MAX_SPEED ]
do

until [  $seed -gt $num_sim ]
do

#home/relax/Ken/simulation/VanetMobism/samples/gen_mobility.sh   $node $simulations_time $seed $num_sim





echo " speed:	${speed}	seed:	 ${seed}"




let seed=seed+1
done






seed=1
if [ speed=0 ]
	then speed=speed+$INTERVAL_1
elif [ speed=1]
	then speed=speed+$INTERVAL_4
else
	speed=speed+$INTERVAL
fi

done


#./simulation_speed.sh