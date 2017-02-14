MINV=$1
MAXV=$2 
FILE=$3

COUNT=$1


	grep "AGT" $FILE > tmp 
	grep "r 3" $FILE >> tmp 
	grep "r 40" $FILE >> tmp 
until [ $COUNT -gt $MAXV ]; do

        grep " --- ${COUNT} " tmp >> tmp1
	let COUNT=COUNT+1
done

rm tmp

	

