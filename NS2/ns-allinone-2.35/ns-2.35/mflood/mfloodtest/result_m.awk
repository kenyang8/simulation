BEGIN {
	recv1=0;
	send=0;
	recv2=0;

	temp = -1;
	total_node=-1;
	hop=0;
	hop_count=0;
	drop = 0;
	forward = 0;
	

}
{
	if(( $1 == "s") && ($7 == "cbr") && ( $4=="AGT" ))
	{
		send_time1[$6] = $2;
		send++;
		recv[$6] = 0;
		
		
	}
	if(( $1 == "r") && ($7 == "cbr")&& ( $4=="AGT" )&&(recv[$6] != 1))
	{
		
		duration_time1 = duration_time1 + $2 - send_time1[$6];
		recv1++;
		recv[$6] = 1;

		hop = hop + $16; 
		
	}


	if(( $1 == "D") && ($4 == "IFQ"))#&&(d[$6]!=1)&&($5=="LOOP"))#&&($5!="LOOP")) #all nodes receive .
	{
		
		drop++;
		#d[$6]=1;
		 
		
	}

	if ( ($1 == "M" )&&( $3 > 0 )) 
	{	
			
		
		total_node =$3 ;
					
	}

	if(( $1 == "r") && ($7 == "cbr")&&(cover[$3] != 1))
	{
		
		recv2++;
		cover[$3] = 1;


			
	}

	if(($1 == "f")&&($7 == "cbr"))
	{
		forward ++;

	}







	
}

END {	
	hop_count = 30-(hop/recv1); #mflood define ttl :30

if(recv1 !=0){
	
	printf("==========================================================\n");
	
	printf("send:  %d	receive:  %d	\n",send,recv1);
	printf("r/s Ratio:	%.4f\n",recv1/send);  
	printf("delay:	%.3f\n",(duration_time1/recv1));
	printf("delay time:	%.3f\n",duration_time1);
	printf("hop count:	%.3f\n",hop_count);
	printf("IFQ drop:	%d\n",drop);
	printf("Total Packets Transmitted:	%d\n",forward);
	printf("Coverage:	%f \n",recv2/total_node);
	printf("total_node:  %d   recv2:   %.3f	\n",(total_node+1),recv2);	
	
	}
else {
	printf("=========================   No receive packet GGGGGGG ==========================\n");	
	printf("send:  %d	receive:  %d	",send,recv1);
	
	}

	
	
}
