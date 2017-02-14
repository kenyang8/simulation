MINV=$1
MAXV=$2 
FILE=$3

COUNT=$1
until [ $COUNT -gt $MAXV ]; do

 
	grep -v "ns_ " $FILE | grep "node_("$COUNT") " >> mobility_new.tcl
	let COUNT=COUNT+1
done 
COUNT=$1 
until [ $COUNT -gt $MAXV ]; do

 
	grep "ns_ " $FILE | grep "node_("$COUNT") " >> tmp
	let COUNT=COUNT+1
done

	sort -t ' ' -nk3 tmp >> mobility_new.tcl

rm tmp