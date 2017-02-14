BEGIN {
	recv=0;
	duration_time=0;
	maxdelay=0;
	mindelay=999;
	maxtime;
	mintime;
}
{
	if( $1 == "enqueue")
	{
		send_time[$2] = $4;
	}
	if( $1 == "dequeue" &&($4 - send_time[$2] > 0.5))
	{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time = duration_time + $4 - send_time[$2];
		recv++;
		if(($4 - send_time[$2])> maxdelay)
		{
			maxdelay = $4 - send_time[$2];
			maxtime = $2;
			#printf("maxdelay is dequeue %d\n",recv);
		}
		if(($4 - send_time[$2])< mindelay)
		{
			mindelay = $4 - send_time[$2];
			mintime = $2;
			#printf("mindelay is dequeue %d\n",recv);
		}
	}

}
END {

	printf("total duration time: %.3f total dequeue count: %d\n",duration_time,recv);
	printf("average duration: %.3f\n",(duration_time/recv));
	printf("maxdelay: %.3f\n",maxdelay);
	printf("mindelay: %.3f\n",mindelay);
}
