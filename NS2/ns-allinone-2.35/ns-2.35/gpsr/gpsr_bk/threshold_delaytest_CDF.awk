BEGIN {
	recv1=0;
	recv2=0;
	recv3=0;
	recv4=0;
	send=0;
	d500=0;
	d1000=0;
	d1500=0;
	d2000=0;
	d2500=0;
	d3000=0;
	d3500=0;
	d4000=0;
	d4500=0;
	d5000=0;
	d5500=0;
	d6000=0;
	d6500=0;
	d7000=0;
	d7500=0;
	d8000=0;
	d8500=0;
	d9000=0;
	d9500=0;
	d10000=0;
	d20000=0;
	d30000=0;
	d40000=0;
	d50000=0;
	d50=0;
	d100=0;
	d200=0;
	d300=0;
	d400=0;
	
}
{
	if(( $1 == "s") && ($7 == "cbr") && ($3=="_6_")&& ( $4=="AGT" )  )
	{
		send++;
		send_time1[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_4_"))
	{
		if($2 - send_time1[$6]<200.5)
		{		
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time1 = duration_time1 + $2 - send_time1[$6];
		recv1++;}
		if($2 - send_time1[$6]<0.05)
		{d50++;}
		if($2 - send_time1[$6]<0.1)
		{d100++;}
		if($2 - send_time1[$6]<0.2)
		{d200++;}
		if($2 - send_time1[$6]<0.3)
		{d300++;}
		if($2 - send_time1[$6]<0.4)
		{d400++;}
		if($2 - send_time1[$6]<0.5)
		{d500++;}
		if($2 - send_time1[$6]<1.0)
		{d1000++;}
		if($2 - send_time1[$6]<1.5)
		{d1500++;}
		if($2 - send_time1[$6]<2.0)
		{d2000++;}
		if($2 - send_time1[$6]<2.5)
		{d2500++;}
		if($2 - send_time1[$6]<3.0)
		{d3000++;}
		if($2 - send_time1[$6]<3.5)
		{d3500++;}
		if($2 - send_time1[$6]<4.0)
		{d4000++;}
		if($2 - send_time1[$6]<4.5)
		{d4500++;}
		if($2 - send_time1[$6]<5.0)
		{d5000++;}
		if($2 - send_time1[$6]<5.5)
		{d5500++;}
		if($2 - send_time1[$6]<6.0)
		{d6000++;}
		if($2 - send_time1[$6]<6.5)
		{d6500++;}
		if($2 - send_time1[$6]<7.0)
		{d7000++;}
		if($2 - send_time1[$6]<7.5)
		{d7500++;}
		if($2 - send_time1[$6]<8.0)
		{d8000++;}
		if($2 - send_time1[$6]<8.5)
		{d8500++;}
		if($2 - send_time1[$6]<9.0)
		{d9000++;}
		if($2 - send_time1[$6]<9.5)
		{d9500++;}
		if($2 - send_time1[$6]<10.0)
		{d10000++;}
		if($2 - send_time1[$6]<20.0)
		{d20000++;}
		if($2 - send_time1[$6]<30.0)
		{d30000++;}
		if($2 - send_time1[$6]<40.0)
		{d40000++;}
		if($2 - send_time1[$6]<50.0)
		{d50000++;}
	}

	if(( $1 == "s") && ($7 == "cbr") && ($3=="_24_")&& ( $4=="AGT" ) )
	{
		send++;
		send_time2[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_20_"))
	{
		if($2 - send_time2[$6]<200.5)
		{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time2 = duration_time2 + $2 - send_time2[$6];
		recv2++;
		}
		if($2 - send_time2[$6]<0.05)
		{d50++;}
		if($2 - send_time2[$6]<0.1)
		{d100++;}
		if($2 - send_time2[$6]<0.2)
		{d200++;}
		if($2 - send_time2[$6]<0.3)
		{d300++;}
		if($2 - send_time2[$6]<0.4)
		{d400++;}
		if($2 - send_time2[$6]<0.5)
		{d500++;}
		if($2 - send_time2[$6]<1.0)
		{d1000++;}
		if($2 - send_time2[$6]<1.5)
		{d1500++;}
		if($2 - send_time2[$6]<2.0)
		{d2000++;}
		if($2 - send_time2[$6]<2.5)
		{d2500++;}
		if($2 - send_time2[$6]<3.0)
		{d3000++;}
		if($2 - send_time2[$6]<3.5)
		{d3500++;}
		if($2 - send_time2[$6]<4.0)
		{d4000++;}
		if($2 - send_time2[$6]<4.5)
		{d4500++;}
		if($2 - send_time2[$6]<5.0)
		{d5000++;}
		if($2 - send_time2[$6]<5.5)
		{d5500++;}
		if($2 - send_time2[$6]<6.0)
		{d6000++;}
		if($2 - send_time2[$6]<6.5)
		{d6500++;}
		if($2 - send_time2[$6]<7.0)
		{d7000++;}
		if($2 - send_time2[$6]<7.5)
		{d7500++;}
		if($2 - send_time2[$6]<8.0)
		{d8000++;}
		if($2 - send_time2[$6]<8.5)
		{d8500++;}
		if($2 - send_time2[$6]<9.0)
		{d9000++;}
		if($2 - send_time2[$6]<9.5)
		{d9500++;}
		if($2 - send_time2[$6]<10.0)
		{d10000++;}
		if($2 - send_time2[$6]<20.0)
		{d20000++;}
		if($2 - send_time2[$6]<30.0)
		{d30000++;}
		if($2 - send_time2[$6]<40.0)
		{d40000++;}
		if($2 - send_time2[$6]<50.0)
		{d50000++;}
	}

	if(( $1 == "s") && ($7 == "cbr") && ($3=="_81_")&& ( $4=="AGT" ) )
	{
		send++;
		send_time3[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_59_"))
	{
		if($2 - send_time3[$6]<200.5)
		{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time3 = duration_time3 + $2 - send_time3[$6];
		recv3++
		}
		if($2 - send_time3[$6]<0.05)
		{d50++;}
		if($2 - send_time3[$6]<0.1)
		{d100++;}
		if($2 - send_time3[$6]<0.2)
		{d200++;}
		if($2 - send_time3[$6]<0.3)
		{d300++;}
		if($2 - send_time3[$6]<0.4)
		{d400++;}
		if($2 - send_time3[$6]<0.5)
		{d500++;}
		if($2 - send_time3[$6]<1.0)
		{d1000++;}
		if($2 - send_time3[$6]<1.5)
		{d1500++;}
		if($2 - send_time3[$6]<2.0)
		{d2000++;}
		if($2 - send_time3[$6]<2.5)
		{d2500++;}
		if($2 - send_time3[$6]<3.0)
		{d3000++;}
		if($2 - send_time3[$6]<3.5)
		{d3500++;}
		if($2 - send_time3[$6]<4.0)
		{d4000++;}
		if($2 - send_time3[$6]<4.5)
		{d4500++;}
		if($2 - send_time3[$6]<5.0)
		{d5000++;}
		if($2 - send_time3[$6]<5.5)
		{d5500++;}
		if($2 - send_time3[$6]<6.0)
		{d6000++;}
		if($2 - send_time3[$6]<6.5)
		{d6500++;}
		if($2 - send_time3[$6]<7.0)
		{d7000++;}
		if($2 - send_time3[$6]<7.5)
		{d7500++;}
		if($2 - send_time3[$6]<8.0)
		{d8000++;}
		if($2 - send_time3[$6]<8.5)
		{d8500++;}
		if($2 - send_time3[$6]<9.0)
		{d9000++;}
		if($2 - send_time3[$6]<9.5)
		{d9500++;}
		if($2 - send_time3[$6]<10.0)
		{d10000++;}
		if($2 - send_time3[$6]<20.0)
		{d20000++;}
		if($2 - send_time3[$6]<30.0)
		{d30000++;}
		if($2 - send_time3[$6]<40.0)
		{d40000++;}
		if($2 - send_time3[$6]<50.0)
		{d50000++;}
	}

	if(( $1 == "s") && ($7 == "cbr") && ($3=="_129_")&& ( $4=="AGT" ) )
	{
		send++;
		send_time4[$6] = $2;
	}
	if(( $1 == "r") && ($7 == "cbr") && ($3=="_26_"))
	{
		if($2 - send_time4[$6]<200.5)
		{
		#printf("snedtime1:%f\n",send_time1[$6]);
		duration_time4 = duration_time4 + $2 - send_time4[$6];

		recv4++;
		}
		if($2 - send_time4[$6]<0.05)
		{d50++;}
		if($2 - send_time4[$6]<0.1)
		{d100++;}
		if($2 - send_time4[$6]<0.2)
		{d200++;}
		if($2 - send_time4[$6]<0.3)
		{d300++;}
		if($2 - send_time4[$6]<0.4)
		{d400++;}
		if($2 - send_time4[$6]<0.5)
		{d500++;}
		if($2 - send_time4[$6]<1.0)
		{d1000++;}
		if($2 - send_time4[$6]<1.5)
		{d1500++;}
		if($2 - send_time4[$6]<2.0)
		{d2000++;}
		if($2 - send_time4[$6]<2.5)
		{d2500++;}
		if($2 - send_time4[$6]<3.0)
		{d3000++;}
		if($2 - send_time4[$6]<3.5)
		{d3500++;}
		if($2 - send_time4[$6]<4.0)
		{d4000++;}
		if($2 - send_time4[$6]<4.5)
		{d4500++;}
		if($2 - send_time4[$6]<5.0)
		{d5000++;}
		if($2 - send_time4[$6]<5.5)
		{d5500++;}
		if($2 - send_time4[$6]<6.0)
		{d6000++;}
		if($2 - send_time4[$6]<6.5)
		{d6500++;}
		if($2 - send_time4[$6]<7.0)
		{d7000++;}
		if($2 - send_time4[$6]<7.5)
		{d7500++;}
		if($2 - send_time4[$6]<8.0)
		{d8000++;}
		if($2 - send_time4[$6]<8.5)
		{d8500++;}
		if($2 - send_time4[$6]<9.0)
		{d9000++;}
		if($2 - send_time4[$6]<9.5)
		{d9500++;}
		if($2 - send_time4[$6]<10.0)
		{d10000++;}
		if($2 - send_time4[$6]<20.0)
		{d20000++;}
		if($2 - send_time4[$6]<30.0)
		{d30000++;}
		if($2 - send_time4[$6]<40.0)
		{d40000++;}
		if($2 - send_time4[$6]<50.0)
		{d50000++;}
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
	printf("test d50: %d  %.3f\n",d50,(d50/recv));
	printf("test d100: %d  %.3f\n",d100,(d100/recv));
	printf("test d200: %d  %.3f\n",d200,(d200/recv));
	printf("test d300: %d  %.3f\n",d300,(d300/recv));
	printf("test d400: %d  %.3f\n",d400,(d400/recv));
	printf("test d500: %d  %.3f\n",d500,(d500/recv));
	printf("test d1000: %d  %.3f\n",d1000,(d1000/recv));
	printf("test d1500: %d  %.3f\n",d1500,(d1500/recv));
	printf("test d2000: %d  %.3f\n",d2000,(d2000/recv));
	printf("test d2500: %d  %.3f\n",d2500,(d2500/recv));
	printf("test d3000: %d  %.3f\n",d3000,(d3000/recv));
	printf("test d3500: %d  %.3f\n",d3500,(d3500/recv));
	printf("test d4000: %d  %.3f\n",d4000,(d4000/recv));
	printf("test d4500: %d  %.3f\n",d4500,(d4500/recv));
	printf("test d5000: %d  %.3f\n",d5000,(d5000/recv));
	printf("test d5500: %d  %.3f\n",d5500,(d5500/recv));
	printf("test d6000: %d  %.3f\n",d6000,(d6000/recv));
	printf("test d6500: %d  %.3f\n",d6500,(d6500/recv));
	printf("test d7000: %d  %.3f\n",d7000,(d7000/recv));
	printf("test d7500: %d  %.3f\n",d7500,(d7500/recv));
	printf("test d8000: %d  %.3f\n",d8000,(d8000/recv));
	printf("test d8500: %d  %.3f\n",d8500,(d8500/recv));
	printf("test d9000: %d  %.3f\n",d9000,(d9000/recv));
	printf("test d9500: %d  %.3f\n",d9500,(d9500/recv));
	printf("test d10000: %d  %.3f\n",d10000,(d10000/recv));
	printf("test d20000: %d  %.3f\n",d20000,(d20000/recv));
	printf("test d30000: %d  %.3f\n",d30000,(d30000/recv));
	printf("test d40000: %d  %.3f\n",d40000,(d40000/recv));
	printf("test d50000: %d  %.3f\n",d50000,(d50000/recv));

}
