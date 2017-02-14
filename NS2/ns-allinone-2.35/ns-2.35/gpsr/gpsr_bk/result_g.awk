#20160206
BEGIN {
	recv_des=0;
	send=0;	
	recv_all=0;	
	total_node=-1;
	hop_des=0;
	hop_all=0;
	hop_count=0;
	drop_IFQ = 0;
	forward = 0;
	all_durantion_time = 0;
	des_duration_time=0;
	delay_des=0;
	delay_all=0;	
	

}
{
	if(( $1 == "s") && ($7 == "cbr") && ( $4=="AGT" ))
	{
		send_time = $2;
		send++;
		recv_des_[$3] = 1;
		recv_all_[$3] = 1;		
		#printf("send_time: %.3f\n",send_time);	
				
		
	}
	if(( $1 == "r") && ($7 == "cbr")&&( $4=="AGT" )&&(recv_des_[$3]!=1))
	{
		
		
		des_duration_time=des_duration_time+$2-send_time;
		recv_des++;		
		recv_des_[$3] = 1;
		hop_des = hop_des + $19; 
		
		#printf("recv:\n",$2);

		
	}

	if(( $1 == "D") && ($4 == "IFQ"))
	{
		drop_IFQ++;
		#d[$6]=1; 
		
	}

	if ( ($1 == "M" )&&(total_node_[$3] != 1)&&( $3 != "nn" )&&( $3!="sc" )&&( $3!="prop" )) 
	{		
		total_node_[$3] = 1;
		total_node ++ ;	
		printf("%s\n",$3);				
	}

	if(($1 == "f")&&($7 == "cbr"))
	{
		forward ++;
	}

	if(( $1 == "r") && ($7 == "cbr")&&(recv_all_[$3] != 1))
	{
		
		all_durantion_time = all_durantion_time+$2-send_time;
		recv_all++;
		recv_all_[$3] = 1;		
		hop_all = hop_all + $19;			
	}	
}

END {	
	hop_count_des = 0; 
	hop_count_all = (hop_all/recv_all);

	if(recv_des!=0){	
		delay_des =des_duration_time/recv_des;
		hop_count_des = (hop_des/recv_des);
	}
	if(recv_all !=0){
		delay_all=(all_durantion_time/recv_all);
	}	
	printf("=======================================\n");	
	printf("send:  %d	recv_des:  %d	\n",send,recv_des); 
	printf("Delay_des:		%.3f\n",delay_des);
	printf("hop_des count:		%.3f\n",hop_count_des);
	printf("Total receive nodes:	%d\n",recv_all);	
	printf("Delay for all nodes:	%.3f\n",delay_all);
	printf("hop_all count:		%.3f\n",hop_count_all);			
	printf("Total nodes forward:	%d\n",forward);
	printf("Coverage:		%.3f  \n",recv_all/total_node);
	printf("Total nodes: 		%d   \n",total_node+1);
	printf("IFQ drop:		%d\n",drop_IFQ);
	printf("Total delay time:	%.3f\n",all_durantion_time);	
}
