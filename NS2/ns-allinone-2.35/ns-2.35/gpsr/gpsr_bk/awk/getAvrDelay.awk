BEGIN {
	sum=0;
	num=0;
	node=0;
}
$0 ~/node/ {
	node=$2;
}
$0 !~/^n.*/ {
	sum += $1;
	num++;
}
END {
	printf "%d %.4f\n",node,sum/num;
}