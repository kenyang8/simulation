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
	duration_time1 = 0;
	
	src ="_576_" ;  
	des = "_577_" ; 
	#輸入來源#輸入目的 
}
{
	if(( $1 == "s") && ($7 == "cbr") && ( $4=="AGT" ))
	{
		send_time1[$6] = $2;  # $6 :封包編號
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
	if(( $1 == "D") && ($4 == "IFQ"))#&&(d[$6]!=1)&&($5=="LOOP"))#&&($5!="LOOP")) #all nodes receive .
	{
		
		drop++;
		#d[$6]=1;
		 
		
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
	

	#th = recv3*512*8/(1000*1000) ;
	#th1 = recv4*512*8/(1000*1000) ;
	duration_time = duration_time1+duration_time2+duration_time3+duration_time4;
if(recv1 != 0){	
	printf("======================================================================\n");
	hop_count = 32-(hop/recv1);
	printf("send:  %d	receive:  %d	\n",recv2,recv1);
	printf("r/s Ratio:	%.4f\n",recv1/recv2);  
	printf("delay:	%.3f\n",(duration_time1/recv1));
	printf("hop count:	%.3f\n",(hop_count))	;
	printf("IFQ drop:	%d\n",drop);
	printf("total delay time:	%.3f\n",duration_time1);
	printf("total recv(AGT):		%d 	\n",recv5);
	printf("total recv(ALL):		%d 	\n",recv6);
	#printf("throughput(不重複):	%f	\n",th);
	#printf("throughput:	%f	\n",th1);
}

else {
	printf("=========================   No receive packet GGGGGGG ==========================\n");	
	printf("send:  %d	receive:  %d	",recv2,recv1);
	
	}
}
