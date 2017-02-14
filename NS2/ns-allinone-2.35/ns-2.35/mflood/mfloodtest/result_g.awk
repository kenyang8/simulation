BEGIN {
	recv1=0;
	send=0;
	recv2=0;
	recv3=0;

	temp = -1;
	total_node=-1;
	hop=0;
	hop_count=0;
	drop = 0;
	forward = 0;
	duration_time1 = 0;
	des_duration_time=0;
	destination_delay=0;
	delay=0;	

}
{
	if(( $1 == "s") && ($7 == "cbr") && ( $4=="AGT" ))
	{
		send_time1 = $2;
		send++;
		recv[$6] = 0;
		printf("send_time1: %.3f\n",send_time1);

		
		
	}
	if(( $1 == "r") && ($7 == "cbr")&&( $4=="AGT" )&&(recv[$6]!=1))
	{
		
		duration_time1 = duration_time1 + $2 - send_time1;
		des_duration_time=des_duration_time+$2-send_time1;
		recv1++;
		recv3++;
		recv[$6] = 1;

		hop = hop + $16; 
		#printf("1\n");
		#printf("recv:\n",$2);
		
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

	if(( $1 == "r") && ($7 == "cbr")&&(node_recv[$3] != 1))
	{
		
		duration_time1 = duration_time1+$2-send_time1;
		recv3++;
		node_recv[$3] = 1;
		temp = $6;		
	}	
}

END {	
	hop_count = 0; #mflood define ttl :30   gpsr: 32

	if(recv1!=0){	
		destination_delay =des_duration_time/recv1;
		hop_count = 32-(hop/recv1);
	}
	if(recv3!=0){
		delay=(duration_time1/recv3);
	}
	
	printf("=======================================\n");	
	printf("send:  %d	receive:  %d	\n",send,recv1);
	printf("destination delay time:	%.3f\n",destination_delay);
	printf("delay:	%.3f\n",delay);
	printf("all delay time:	%.3f\n",duration_time1);
	printf("total receive:	%.3f\n",recv3);
	printf("hop count:	%.3f\n",hop_count);
	printf("IFQ drop:	%d\n",drop);
	printf("Total Packets Transmitted:	%d\n",forward);
	printf("Coverage:	%.3f \n",recv2/total_node);
	printf("total_node:  %d   recv2:   %.3f	\n",(total_node+1),recv2);
	
}
