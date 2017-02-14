# example :    ./setdest -v 2 -n 50 -s 1 -m 20 -M20 -t 350 -P 1 -p 2 -x 1000 -y 1000 > test/533
VERSION=2
NUM_OF_NODES=50
SPEED_TYPE=2
MIN_SPEED=20
MAX_SPEED=100
SIMULATION_TIME=350
PAUSE_TYPE=1
PAUSE_TIME=2
X=1000
Y=1000

INTERVAL=20
speed=$MIN_SPEED
seed=1
num_sim=50


until [  $speed -gt $MAX_SPEED ]
do

until [  $seed -gt $num_sim ]
do

#./mobgen $NODE $X $Y $simulations_time $speed $SPEED_DELTA  $PAUSE 1  N > n50sp${speed}s${seed}
 ./setdest -v $VERSION -n $NUM_OF_NODES -s $SPEED_TYPE -m $speed -M $speed -t $SIMULATION_TIME -P $PAUSE_TYPE -p $PAUSE_TIME -x $X -y $Y > test/n50sp${speed}s${seed}


let seed=seed+1
done


seed=1

let speed=speed+$INTERVAL
done

until [  $seed -gt $num_sim ]
do

#./mobgen $NODE $X $Y $simulations_time $speed $SPEED_DELTA  $PAUSE 1  N > n50sp${speed}s${seed}
 ./setdest -v $VERSION -n $NUM_OF_NODES -s $SPEED_TYPE -m 0.0001-M $0.00001 -t $SIMULATION_TIME -P $PAUSE_TYPE -p $PAUSE_TIME -x $X -y $Y > test/n50sp0s${seed}


let seed=seed+1
done
