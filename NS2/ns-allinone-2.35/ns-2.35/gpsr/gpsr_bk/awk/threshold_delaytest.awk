BEGIN {
	recv1=0;
	recv2=0;
	recv3=0;
	recv4=0;
	send=0;
}
{
	if(( $1 == "s") && ($7 == "cbr") && ($3=="_6_")&& ( $4=="AGT" )  )
	{
		send++;
		send_time1[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_4_") )
	{
		if($2 - send_time1[$6]<2.0)
		{		
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time1 = duration_time1 + $2 - send_time1[$6];
		recv1++;}
	}

	if(( $1 == "s") && ($7 == "cbr") && ($3=="_24_")&& ( $4=="AGT" ) )
	{
		send++;
		send_time2[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_20_"))
	{
		if($2 - send_time2[$6]<2.0)
		{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time2 = duration_time2 + $2 - send_time2[$6];
		recv2++;
		}
	}

	if(( $1 == "s") && ($7 == "cbr") && ($3=="_81_")&& ( $4=="AGT" ) )
	{
		send++;
		send_time3[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_59_"))
	{
		if($2 - send_time3[$6]<1.0)
		{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time3 = duration_time3 + $2 - send_time3[$6];
		recv3++;
		}
	}

	if(( $1 == "s") && ($7 == "cbr") && ($3=="_129_")&& ( $4=="AGT" ) )
	{
		send++;
		send_time4[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_26_"))
	{
		if($2 - send_time4[$6]<2.0)
		{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time4 = duration_time4 + $2 - send_time4[$6];
		recv4++;
		}
	}
}
END {
	duration_time = duration_time1+duration_time2+duration_time3+duration_time4;
	recv = recv1+recv2+recv3+recv4;
	printf("test1 end: %.3f %d\n",(duration_time1/recv1),recv1);
	printf("test2 end: %.3f %d\n",(duration_time2/recv2),recv2);
	printf("test3 end: %.3f %d\n",(duration_time3/recv3),recv3);
	printf("test4 end: %.3f %d\n",(duration_time4/recv4),recv4);
	printf("test end: %.3f %d\n",(duration_time/recv),recv);
	printf("test send: %d\n",send);
}
