BEGIN {
	recv1=0;
	recv2=0;
	recv3=0;
	recv4=0;
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
{
	if(( $1 == "s") && ($7 == "cbr") && ( $4=="AGT" ))
	{
		send_time1[$6] = $2;
		recv2++;
		recv[$6] = 0;
		th2[$6] = 0;
		d[$6]=0;
		
	}
	if(( $1 == "r") && ($7 == "cbr")&& ( $4=="AGT" )&&(recv[$6] != 1))
	{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time1 = duration_time1 + $2 - send_time1[$6];
		recv1++;
		recv[$6] = 1;
		d[$6] = 1;
		hop = hop + $16; 
		
	}
	if(( $1 == "r") && ($7 == "cbr") && ( $4=="AGT" )&&(th2[$6] != 1))
	{
		
		recv3++;
		th2[$6]=1;
		
	}

	if(( $1 == "r") && ($7 == "cbr")) #all nodes receive .
	{
		
		recv4++;
		
	}
	if(( $1 == "D") && ($7 == "cbr")&&(d[$6]!=1)&&($5=="LOOP"))#&&($5!="LOOP")) #all nodes receive .
	{
		
		drop++;
		d[$6]=1;
		 
		
	}
	if(( $1 == "r") && ($7 == "cbr") && ( $4=="AGT" ))
	{
		recv5++;
	}
	if(( $1 == "r") && ($7 == "cbr"))
	{
		recv6++;
	}
	





	
}

END {	
	hop_count = 30-(hop/recv1);

	th = recv3*512*8/(1000*1000) ;
	th1 = recv4*512*8/(1000*1000) ;
	duration_time = duration_time1+duration_time2+duration_time3+duration_time4;
	printf("======================================================================\n");
	
	printf("send:  %d	receive:  %d	totalrecv:	%d	%d\n",recv2,recv1,recv5,recv6);
	printf("r/s Ratio:	%.4f\n",recv1/recv2);  
	printf("delay:	%.3f\n",(duration_time1/recv1));
	printf("delay time:	%.3f\n",duration_time1);
	printf("hop count:	%.3f\n",(hop_count))	;
	printf("throughput(不重複):	%f	\n",th);
	printf("throughput:	%f	\n",th1);
	
	
}
