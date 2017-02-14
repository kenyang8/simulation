BEGIN {
	init=0;
	start_time=0;
	end_time=0;
	num=0;
}
$0 ~/^s.* MAC/ && $7=="cbr" {
	if(match($14,"\\["src":")>0 && match($15,dst":")==1 && $3=="_"src"_") {
		if(init==0) {
			start_time=$2;
			init=1;
		}
	}
}
$0 ~/^r.* MAC/ && $7=="cbr" {
	if(match($14,"\\["src":")>0 && match($15,dst":")==1 && $3=="_"dst"_") {
		end_time=$2;
		if(end_time!=0) {
			num++;
		}
	}
}
END {
	if(end_time==0) {
		printf "nonexist\n"
	}	else {
		duration=end_time-start_time;
		printf "%.4f\n",duration/num;
	}
}