BEGIN {
	recv1=0;
	recv2=0;
	recv3=0;
	
	recv5=0;
	recv6=0;

	th = 0.0;
	th1= 0.0;
	drop = 0;
	
	hop=0;
	hop_count=0;
	
	src ="_576_" ;  
	des = "_577_" ; 
	#輸入來源#輸入目的 
}
{	split($3,a,"_");
	node = a[2] ;
	if(( $1 == "s")&&($7 == "cbr")&&( $4=="AGT" ))
	{
		send_time1[$6] = $2;
		recv4[$6] = 0;
		recv2++;
		

		
	}
	if(( $1 == "r")&&($7 == "cbr")&&(recv[node,$6] != 1)&&(node>=120)&&(node<=212))#試著paper中計算delay   44-102L10 L14 42-312
	{
	
		duration_time1 = duration_time1 + $2 - send_time1[$6];
		
		recv[node,$6] = 1;
		recv1++;
			
		
	}
	if(( $1 == "r") && ($7 == "cbr")&& ( $4=="AGT" )&&(recv4[$6] != 1))      #實際收到封包不重複
	{
	
		duration_time2 = duration_time2 + $2 - send_time1[$6];
		recv3++;
		recv4[$6] = 1;
	
		hop = hop + $16; 
	}
		
	
	






	
}

END {	
	hop_count = 30-(hop/recv3);
	
	printf("======================================================================\n");
	
	printf("send:  %d	receive:  %d	\n",recv2,recv3);
	printf("r/s Ratio:	%.4f\n",recv3/recv2);  
	
	printf("delay:	%.3f\n",(duration_time1/recv1));
	printf("R:	%d\n",recv1);
	printf("Delay:	%.3f\n",duration_time1);
	printf("delay time(original):	%.3f\n",duration_time2/recv3);
	printf("hop count:	%.3f\n",hop_count);
	
	
	
	
}
