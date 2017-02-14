BEGIN {
	sum=0;
	num=0;
	node=0;
	nonum=0;
}
$0 ~/node/ {
	node=$2;
}
$0 !~/^n.*/ {
	sum += $1;
	num++;
}
$0 ~/nonexist/ {
	nonum++;
}
END {
	AvrRate=1-nonum/sum;
	printf "%d %.4f\n",node,(sum/num)*(conn*AvrRate);
}