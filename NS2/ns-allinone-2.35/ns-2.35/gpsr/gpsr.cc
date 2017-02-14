/* -*- Mode:C++; c-basic-offset: 2; tab-width:2, indent-tabs-width:t -*- 
 * Copyright (C) 2005 State University of New York, at Binghamton
 * All rights reserved.
 *
 * NOTICE: This software is provided "as is", without any warranty,
 * including any implied warranty for merchantability or fitness for a
 * particular purpose.  Under no circumstances shall SUNY Binghamton
 * or its faculty, staff, students or agents be liable for any use of,
 * misuse of, or inability to use this software, including incidental
 * and consequential damages.

 * License is hereby given to use, modify, and redistribute this
 * software, in whole or in part, for any commercial or non-commercial
 * purpose, provided that the user agrees to the terms of this
 * copyright notice, including disclaimer of warranty, and provided
 * that this copyright notice, including disclaimer of warranty, is
 * preserved in the source code and documentation of anything derived
 * from this software.  Any redistributor of this software or anything
 * derived from this software assumes responsibility for ensuring that
 * any parties to whom such a redistribution is made are fully aware of
 * the terms of this license and disclaimer.
 *
 * Author: Ke Liu, CS Dept., State University of New York, at Binghamton 
 * October, 2005
 *
 * GPSR code for NS2 version 2.26 or later
 * Note: this implementation of GPSR is different from its original 
 *       version wich implemented by Brad Karp, Harvard Univ. 1999
 *       It is not guaranteed precise implementation of the GPSR design
 */

/* gpsr.cc : the definition of the gpsr routing agent class
 *           
 */
#include "gpsr.h"
#include "gpsr-seqtable.h"
#define PI 3.141593
#define DEFAULT_SAFE_DISTANCE 200
#define UPDATE_DATAPACKET
//#define KEN
int hdr_gpsr::offset_;


void mywrite_trace_file (const char * p)
{
  FILE *fp;
  fp = fopen(NB_TRACE_FILE, "a+");
  fputs(p, fp);
  fclose(fp);
}

static class GPSRHeaderClass : public PacketHeaderClass{
public:
  GPSRHeaderClass() : PacketHeaderClass("PacketHeader/GPSR",
					 sizeof(hdr_all_gpsr)){
    bind_offset(&hdr_gpsr::offset_);
  }
}class_gpsrhdr;

static class GPSRAgentClass : public TclClass {
public:
  GPSRAgentClass() : TclClass("Agent/GPSR"){}
  TclObject *create(int, const char*const*){
    return (new GPSRAgent());
  }
}class_gpsr;

void
GPSRHelloTimer::expire(Event *e){
  a_->hellotout();
}
void
GPSRCarryTimer::expire(Event *e){
  a_->carryout();
}
void
GPSRQueryTimer::expire(Event *e){
  a_->querytout();
}

void
GPSRUpdateSinkLocTimer::expire(Event *e){
a_->updatesinkloctout();
}

void
GPSRAgent::hellotout(){
  getLoc();
  getmyspeed();
  getmyangle();
  nblist_->myinfo(my_id_, my_x_, my_y_,my_speed_,my_angle_); // 
  //hellomsg();
  hello_timer_.resched(hello_period_);
  nblist_->delalltimeout();
  
}

void
GPSRAgent::carryout(){
	//printf("node %d carry timer test success\n",my_id_); 確定成功,無須在測試了
	//carry_timer_.resched(carry_period);
	int result = 0;
	//printf("node %d enter carryout function at %f\n",my_id_,GPSR_CURRENT);
	result = sendCarry();
	switch (result)
	{
		case 2:
		 printf("node %d enter case 2 function at %f\n",my_id_,GPSR_CURRENT);
		 carry_timer_.resched(carry_period_);
		 break;
		case 1:
		 printf("node %d enter case 1 function at %f\n",my_id_,GPSR_CURRENT);
		 carry_timer_.resched(0.1);
		 break;
		case 0:
		default:
		 printf("node %d enter case 0 function at %f\n",my_id_,GPSR_CURRENT);
		 carry_timer_.resched(INFINITE_DELAY);	 
	}
		
}
void 
GPSRAgent::updatesinkloctout(){
  getLoc();
  getmyspeed();
  getmyangle();
  sink_list_->update_sink_loc(my_id_,my_x_,my_y_);
 // printf("__________________________________\n");
  update_sink_loc_timer_.resched(update_sink_loc_period_);
}

void
GPSRAgent::startSink(){
  // modify by anzizhao
 
  // 修改queryout（）函数
   getLoc();
   getmyspeed();
   getmyangle();
   
   if(sink_list_->new_sink(my_id_, my_x_, my_y_,
                            my_id_, 0, query_counter_,my_speed_,my_angle_))
   {
     query(my_id_);
     query_counter_++;
     query_timer_.resched(query_period_);

   }
   
   //     querytout();
  //querytout();
  
}

void
GPSRAgent::startSink(double gp){
  query_period_ = 1.0;
  
  sink_list_->setPeriod(query_period_,delperiod, usefulperiod);

  startSink();
}

void
GPSRAgent::querytout()
{
  getLoc();
  getmyspeed();
  getmyangle();

  struct sink_entry temp ;
  temp.id_ = my_id_;
  temp.x_  = my_x_ ;
  temp.y_  = my_y_ ;
  temp.lasthop_ =  0;
  temp.hops_ =  0;
  temp.seqno_ =  query_counter_;
  temp.speed_ = my_speed_;
  temp.angle_ = my_angle_;
  temp.ts_ = GPSR_CURRENT;
  
  sink_list_->updateLocbyID(temp);
  //printf("node %d is going to test remove_timeout function\n",my_id_);
  //printf("now location is: %f %f\n",temp.x_,temp.y_); //節點自己xy座標資訊在這邊並無異常,但其他節點就沒有保證了 
  sink_list_->remove_timeout();
  
  query(my_id_);
  query_counter_++;
     
 // sink_list_->sink_test();
  query_timer_.resched(query_period_);
}



void
GPSRAgent::getLoc(){
  GetLocation(&my_x_, &my_y_);
}

void
GPSRAgent::GetLocation(double *x, double *y){
  double pos_x_, pos_y_, pos_z_;
  node_->getLoc(&pos_x_, &pos_y_, &pos_z_);
  *x=pos_x_;
  *y=pos_y_;
}
void
GPSRAgent::getmyspeed(){

  if((GPSR_CURRENT - time_record) < 0.1)
      return;
      
  //printf("node: %d my_x: %f my_y_: %f x_temp: %f y_temp: %f\n",my_id_,my_x_,my_y_,x_temp,y_temp);
  double tempx = my_x_ - x_temp;
  double tempy = my_y_ - y_temp;
 // printf("tempx: %f tempy: %f\n",tempx,tempy);
  tempx = tempx * tempx;
  tempy = tempy * tempy;
  //printf("time period: %f\n",(GPSR_CURRENT - time_record));
  my_speed_ = sqrt(tempx + tempy)/(GPSR_CURRENT - time_record);
  time_record = GPSR_CURRENT;
}

void
GPSRAgent::getmyangle(){
  if((my_x_ == x_temp) && (my_y_ == y_temp))
	return;
  double tempx = my_x_ - x_temp;
  double tempy = my_y_ - y_temp;
  //printf("anglex: %f angley: %f\n",tempx,tempy);
  my_angle_ = atan2(tempy,tempx) * 180 / PI;
  x_temp = my_x_;
  y_temp = my_y_;
  //printf("node %d angle: %f\n",my_id_,my_angle_);
}

GPSRAgent::GPSRAgent() : Agent(PT_GPSR), 
                         hello_timer_(this), query_timer_(this),update_sink_loc_timer_(this),
                         carry_timer_(this), my_id_(-1), my_x_(0.0), my_y_(0.0),
                         recv_counter_(0), query_counter_(1),    // modify by anzizhao 
                         query_period_(INFINITE_DELAY),
                         safe_distance_(DEFAULT_SAFE_DISTANCE)   //add by anzizhao 
{
  bind("planar_type_", &planar_type_); //planar_type_ ==1  (run.tcl裡的設定值,也是預設值) 
  bind("hello_period_", &hello_period_);
  bind("carry_period_", &carry_period_);
  bind("safe_distance_",&safe_distance_ );
  bind("delperiod", &delperiod);
  bind("usefulperiod", &usefulperiod);
  bind("update_sink_loc_period_",&update_sink_loc_period_);
  
  sink_list_ = new Sinks();
  nblist_ = new GPSRNeighbors();
  //GPSRPacketList_ = new GPSRBroadcast();
  broadcast_buffer GPSRPacketList_ ;
  my_buffer_ = NULL;
  for(int i=0; i<5; i++)
    randSend_.reset_next_substream();
   int myseq_ = 0;
  EchoT= new Echo_Timeout(this);
  p2pSendEvent=0;

}
  
GPSRAgent::~GPSRAgent()
{
  delete sink_list_;
  delete nblist_;
  delete GPSRPacketList_;
  struct carry_buffer *temp = my_buffer_;
  struct carry_buffer *del = NULL;
  while ( temp )
  {
    del = temp;
    temp = temp->next_;
    free(del);
      
  }
  my_buffer_ = NULL;
}
 
void
GPSRAgent::turnon(){
  getLoc();
  my_speed_ = 0.0;
  my_angle_ = 0.0;
  nblist_->myinfo(my_id_, my_x_, my_y_,my_speed_,my_angle_);
  x_temp = my_x_;
  y_temp = my_y_;
  //printf("node %d turnon anf x_temp: %f y_temp: %f\n",my_id_,x_temp,y_temp);
  time_record = GPSR_CURRENT;
  isturnon = true;
  my_buffer_= NULL;
  carry_and_forward = false;
  carry_counter = 10;
  nblist_->SetSafetyDis(safe_distance_);
  nblist_->SetHelloPeriod(hello_period_);

  carry_timer_.resched(carry_period_);
  hello_timer_.resched(randSend_.uniform(0.0, 0.5)); 
  update_sink_loc_timer_.resched(start_update_time_);
 // printf("%d turn on at %f\n",my_id_,GPSR_CURRENT);
}

void
GPSRAgent::turnoff(){
  hello_timer_.resched(INFINITE_DELAY);
  query_timer_.resched(INFINITE_DELAY);
  update_sink_loc_timer_.resched(INFINITE_DELAY);
  // sink_list_->clear();
  // nblist_->clear();
    //    if  sink_list_->updateLocbyID(my_id_, my_x_, my_y_, 0)
  // 
  
  isturnon = false;
  
 // printf("%d turn off at %f\n",my_id_,GPSR_CURRENT);
}

void 
GPSRAgent::hellomsg(){
  if(my_id_ < 0) return;    
  if ( ! isturnon ) return ;
  
  Packet *p = allocpkt();
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  struct hdr_gpsr_hello *ghh = HDR_GPSR_HELLO(p);

  cmh->next_hop_ = IP_BROADCAST;
  cmh->last_hop_ = my_id_;
  cmh->addr_type_ = NS_AF_INET;
  cmh->ptype() = PT_GPSR;
  cmh->size() = IP_HDR_LEN + ghh->size();

  iph->daddr() = IP_BROADCAST;
  iph->saddr() = my_id_;
  iph->sport() = RT_PORT;
  iph->dport() = RT_PORT;
  iph->ttl_ = IP_DEF_TTL;

  ghh->type_ = GPSRTYPE_HELLO;
  ghh->x_ = (float)my_x_;
  ghh->y_ = (float)my_y_;
  ghh->speed_ = my_speed_;
  ghh->angle_ = my_angle_;

  send(p, 0);
}


void
GPSRAgent::query(nsaddr_t id){
  if(my_id_ < 0) return;
  if ( ! isturnon ) return ;
  Packet *p = allocpkt();

  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  struct hdr_gpsr_query *gqh = HDR_GPSR_QUERY(p);

  cmh->next_hop_ = IP_BROADCAST;
  // 紀錄上一跳節點
  cmh->last_hop_ = my_id_;
  cmh->addr_type_ = NS_AF_INET;
  cmh->ptype() = PT_GPSR;
  cmh->size() = IP_HDR_LEN + gqh->size();
  
  iph->daddr() = IP_BROADCAST;
  // 紀錄來源節點id
  iph->saddr() = id;//usually is my_id_ because it's execute by a sink node
  iph->sport() = RT_PORT;
  iph->dport() = RT_PORT;
  iph->ttl_ = IP_DEF_TTL;

  gqh->type_ = GPSRTYPE_QUERY;
  double tempx, tempy, ts, temp_speed, temp_angle;
  int hops;
  u_int8_t seqno;
  
   // if (cmh->ptype()!=PT_GPSR)
	//	printf("daddr information: %d at %f in query function\n",iph->daddr(),GPSR_CURRENT);
      //if (cmh->ptype()!=PT_GPSR)
        //{
			sink_list_->getLocbyID(id, tempx, tempy, hops, seqno, ts, temp_speed, temp_angle);
            //printf("act node: %d x-y after function:%f %f \n",my_id_,tempx,tempy); 根本沒進來這if判斷過			
		//}
	//	printf("daddr information in query function after: %d \n",iph->daddr());
   //   if (cmh->ptype()!=PT_GPSR)

  if(tempx > 0.0 && tempy > 0.0){
    gqh->x_ = (float)tempx;
    gqh->y_ = (float)tempy;
    gqh->speed_ = temp_speed;
    gqh->angle_ = temp_angle;
    gqh->hops_ = hops;
  
  }else {
    Packet::free(p);
    return;
  }
  gqh->ts_ = (float)ts;
  cmh->ts_ = (float)ts;
  //gqh->seqno_ = query_counter_;
  if ( my_id_ == id )
    gqh->seqno_ = query_counter_;
  else
    gqh->seqno_ = seqno;
  //printf("query error detection: %f %f\n",gqh->x_,gqh->y_);
  //send(p, 0);
}


//當節點turnoff後, 不再發送hello包
void
GPSRAgent::recvHello(Packet *p){
  struct hdr_cmn *cmh = HDR_CMN(p);//包含packet type, timestamp, prev_hop, next_hop, last_hop
  struct hdr_ip *iph = HDR_IP(p);//包含src_id, dst_id, ttl counter
  struct hdr_gpsr_hello *ghh = HDR_GPSR_HELLO(p);

  nblist_->newNB(cmh->last_hop_, (double)ghh->x_, (double)ghh->y_, (double)ghh->speed_,(double)ghh->angle_);
  struct sink_entry temp ;
   
  temp.id_ = iph->saddr();
  temp.x_  = ghh->x_ ;
  temp.y_  = ghh->y_ ;
  temp.lasthop_ =  cmh->last_hop_ ;
  temp.hops_ =  0;
  temp.seqno_ =  0;
  temp.speed_ = ghh->speed_;
  temp.angle_ = ghh->angle_;
  temp.next_ = NULL;
  
  //add by anzizhao 
  temp.ts_ = GPSR_CURRENT;
  sink_list_->updateLocbyID(temp);
  // if ( (my_id_ == 96 && cmh->last_hop_ == 37) || (my_id_ == 37 && cmh->last_hop_ == 96) ) {
  //   char mesg[100];
  //   sprintf(mesg," time %f id %d(%f,%f) receive hello message from %d(%f,%f)\n",
  //           GPSR_CURRENT,my_id_,my_x_, my_y_, cmh->last_hop_ , ghh->x_, ghh->y_ );
    
    
  //   mywrite_trace_file(mesg);
    
  // }
  
  //trace("%d recv Hello from %d", my_id_, cmh->last_hop_);
  
  // modify by anzizhao 
  Packet::free(p);
  
}

void
GPSRAgent::recvQuery(Packet *p){
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  struct hdr_gpsr_query *gqh = HDR_GPSR_QUERY(p);
  struct sink_entry temp ;
   
  temp.id_ = iph->saddr();
  temp.x_  = gqh->x_ ;
  temp.y_  = gqh->y_ ;
  temp.speed_ = gqh->speed_;
  temp.angle_ = gqh->angle_;
  temp.lasthop_ =  cmh->last_hop_ ;
  temp.hops_ =  1+gqh->hops_;
  temp.seqno_ =  gqh->seqno_;
  temp.next_ = NULL;
 // printf("node %d recieved query about %d at %f\n",my_id_,temp.id_,GPSR_CURRENT);
 // printf("location of this query: %f %f\n",temp.x_,temp.y_);
 // printf("my location now is :%f %f\n\n",my_x_,my_y_);
  //add by anzizhao 
  temp.ts_ = gqh->ts_;
  // if(sink_list_->new_sink(iph->saddr(), gqh->x_, gqh->y_, 
  //   		  cmh->last_hop_, 1+gqh->hops_, gqh->seqno_))
  //   query(iph->saddr());
  //  trace("%d recv Query from %d ", my_id_, iph->saddr());

  // modify 
  if ( sink_list_->new_sink(temp) )
	{
		//printf("recv query function test \n");
		//printf("%d recv Query from %d at %f\n", my_id_, iph->saddr(),GPSR_CURRENT);
		query(iph->saddr());
	}
  
  // chenhuanyin
  Packet::free(p);
  
}

void
GPSRAgent::sinkRecv(Packet *p){
  FILE *fp = fopen(SINK_TRACE_FILE, "a+");
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  //  struct hdr_gpsr_data *gdh = HDR_GPSR_DATA(p);
  
  fprintf(fp, "GPSR_CURRENT:   %2.f\t iph->saddr():  %d\t  cmh->num_forwards():   %d\n", GPSR_CURRENT,
	  iph->saddr(), cmh->num_forwards());
  fclose(fp);
}
void
GPSRAgent::forwardData(Packet *p) {
  struct hdr_cmn *cmh = HDR_CMN(p);//包含packet type, timestamp, prev_hop, next_hop, last_hop
  struct hdr_ip *iph = HDR_IP(p);//包含src_id, dst_id, ttl counter
  //sink_list_->sink_test();
  nsaddr_t nexthop;
  double waiting_time;
  printf("==============In forwardData()==============\n");
  //printf("cmh->direction() =%d,my id is %d\n",cmh->direction(),my_id_);
  
  if(cmh->direction() == hdr_cmn::UP && ((nsaddr_t)iph->daddr() == my_id_)){//這一段if內的判別表示不會在轉傳了
    sinkRecv(p);
    // printf("receive from gpsrup \n");
    //  port_dmux_->recv(p, 0);
    struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);
    struct sink_entry temp ;
    temp.id_ = iph->saddr();
    temp.x_  = gdh->sx_ ;
    temp.y_  = gdh->sy_ ;
    temp.lasthop_ =  cmh->last_hop_ ;
    //   temp.hops_ =  gqh->hops_;

    #define OLDSEQNO  0
    temp.seqno_ = OLDSEQNO ;
    //  temp.next_ = NULL;
  
    //add by anzizhao 
    temp.ts_ = gdh->ts_;
    //printf("forward function before assert command\n");
    assert (sink_list_->updateLocbyID(temp) );
    //printf("forward function before assert command after\n\n");
    // assert(sink_list_->updateLocbyID(iph->saddr(),gdh->sx_,gdh->sy_,0,gdh->ts_));
    Packet::free(p);
    //printf("In forwardData(), iph->daddr() == my_id_\n");
    return;
  }
  else {//以下部份表示要作轉傳的判斷了
    struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);//packet type, source & destination (x,y), time stamp
     getLoc();
     nblist_->myinfo(my_id_, my_x_, my_y_,my_speed_,my_angle_);
    //printf("gdh sx: %f  sy: %f\n",gdh->sx_,gdh->sy_);
    //printf("gdh dx: %f  dy: %f\n",gdh->dx_,gdh->dy_);
    //printf("dst id: %d\n\n",iph->daddr());
    double dx = gdh->dx_;
    double dy = gdh->dy_;
    
    if(gdh->mode_ == GPSR_MODE_GF){
      struct gpsr_neighbor neighbor;
  //    if ( nblist_->getnb(iph->daddr(),neighbor) )
   //     nexthop = neighbor.id_;//如果neighbor裡面有sink的話,就直接傳傳給sink
   //   else{
#ifdef UPDATE_DATAPACKET		  
		//nblist_->NB_test(my_id_);
		//printf("node %d 's sinklist\n",my_id_);
		//sink_list_->sink_test();
		double update_ts = sink_list_->getTimeStamp(iph->daddr());
		if((update_ts - gdh->ts_) > double(update_sink_loc_period_*5)){
			//printf("before setnewlocation x: %f y: %f\n",gdh->dx_,gdh->dy_);
			double tempdx = gdh->dx_;
			double tempdy = gdh->dy_;
			sink_list_->SetNewLocation(iph->daddr(),tempdx,tempdy);
			gdh->dx_ = tempdx;
			gdh->dy_ = tempdy;
			//printf("after setnewlocation x: %f y: %f\n",gdh->dx_,gdh->dy_);
			dx = gdh->dx_;
			dy = gdh->dy_;
		}
    #endif
        //nexthop = nblist_->gf_nexthop1(dx, dy, my_id_,gdh->lx_,gdh->ly_);//找出與sink最近的一個節點（但小於safe_distance_） 
        //如果都不在neighbor_list找到合適的轉傳對象,則nexthop=(-1)
        //nexthop = nblist_->gf_nexthop(dx, dy);


      // waiting_time = nblist_ ->gf_nexthop2(dx, dy, my_id_,gdh->lx_,gdh->ly_,my_speed_,my_angle_);
       waiting_time = nblist_ ->gf_nexthop3(gdh->lx_,gdh->ly_);
      // printf("In forwardData(),after calculate waiting time.\n");
      // printf("my_id_ = %d    gdh->seq_ = %d\n",my_id_,gdh->seq_);

       if (waiting_time == -1)
       {
         FILE *fp = fopen("gpsrnb_trace.tr", "a+"); 
         fprintf (fp, "time %.3f uuid %d src %d(%f,%f) dst %d(%f,%f) my_id %d(%f,%f) nexthop %f\n ",
                  GPSR_CURRENT,cmh->uid_, iph->saddr(),gdh->sx_,gdh->sy_, iph->daddr(), gdh->dx_,
                  gdh->dy_, my_id_, my_x_, my_y_ , nexthop );
         fclose(fp);
         //addCarry(p);
         return;
       }


      cmh->direction() = hdr_cmn::DOWN;
      cmh->addr_type() = NS_AF_INET;
      cmh->next_hop_ = IP_BROADCAST;
      //cmh->next_hop_ = nexthop;
      printf("cmh->last_hop_ = %d\n", cmh->last_hop_);
       cmh->last_hop_ = my_id_;
       gdh->lx_ = my_x_;
       gdh->ly_ = my_y_;
       gdh->ts_ = (float)GPSR_CURRENT;
       printf("Before enter newBroadcastTimer\n");
       printf("my_id_ = %d    gdh->seq_ = %d\n",my_id_,gdh->seq_);
       printf("GPSR_CURRENT: %f\n",GPSR_CURRENT );
       printf("waiting_time: %f\n",waiting_time );
       p2pSendEvent= new Event();  // 第二個不同點
       Scheduler::instance().schedule(EchoT, p2pSendEvent, waiting_time);
       newBroadcastTimer(p,waiting_time);
       Packet::free(p);


	   }  //end if if(gdh->mode_ == GPSR_MODE_GF){

     }//end else
}



void  
GPSRAgent::Recv_callback(Event* e){

   // printf("==============Time out!!! My id : %d ==============\n",my_id_);
   // printf("GPSR_CURRENT: %f\n",GPSR_CURRENT );
   
   find_BroadcastTimer(GPSR_CURRENT);

   //p2pSendEvent =0;

}





void
GPSRAgent::rt_resolve(Packet *p) {
	struct hdr_cmn *cmh = HDR_CMN(p);//包含packet type, timestamp, prev_hop, next_hop, last_hop
	struct hdr_ip *iph = HDR_IP(p);//包含src_id, dst_id, ttl counter
	struct hdr_gpsr_data *gdh = HDR_GPSR_DATA(p);

	gpsr_RTEntry* rt;

	rt = rtable_.rt_lookup(iph->saddr());

   // printf("In rt_resolve,my_id_ is %d,seq_ is %d\n",my_id_,gdh->seq_);

	if(rt == NULL) {
		rt = new gpsr_RTEntry(iph->saddr(), gdh->seq_);

		LIST_INSERT_HEAD(&rtable_.rthead,rt,rt_link);		
	
		//printf("%.8f %d,no uptarget,\n",NOW,index_);
              if (check_Broadcast_seq( gdh->seq_)){
               // printf("In recv(),enter del_Broadcast_seq()\n");
                del_Broadcast_seq(gdh->seq_);
                
                drop(p, "LOWSEQ");

              }             

           // printf("In rt_resolve(),enter forwardData1()\n");
		forwardData(p);
		
//printf("%.8f %d,no rt,so forward.rt_seq:%d,pkt seq:%d\n",NOW,index_,rt->max_seqno,fh->seq_);
rtable_.rt_print();		

#ifdef KEN
	
	  FILE *fp = fopen("rt_solve.tr", "a+"); 
			 fprintf (fp, "time %.3f  src %d(%f,%f) dst %d(%f,%f) my_id %d(%f,%f) NEW TO FORWARD_DATA \n ",
					  GPSR_CURRENT, iph->saddr(),gdh->sx_,gdh->sy_, iph->daddr(), gdh->dx_,
					  gdh->dy_, my_id_, my_x_, my_y_ );
			 fclose(fp);
 #endif

		
	}
//	else if(rt->seq_ < fh->seq_ )
	else if(rt->isNewSeq(gdh->seq_) )
	{
		//printf("%.8f %d,no uptarget,\n",NOW,index_);
              if (check_Broadcast_seq( gdh->seq_)){
                    
                 // printf("In recv(),enter del_Broadcast_seq()\n");
                  del_Broadcast_seq(gdh->seq_);
                  
                  drop(p, "LOWSEQ");

              }              
		
		//printf("In rt_resolve(),enter forwardData2()\n");
             forwardData(p); //NO_DELAY 
//		rt->seq_ = fh->seq_;
		rt->addSeq(gdh->seq_);
             rtable_.rt_print();		


	}
	else        
	{
	    //printf("Drop the packet,LOWSEQ.\n");
           if (check_Broadcast_seq( gdh->seq_)){                   
                  //printf("In recv(),enter del_Broadcast_seq()\n");
                  del_Broadcast_seq(gdh->seq_);
                }
            drop(p, "LOWSEQ");

	}
}




void
GPSRAgent::recv(Packet *p, Handler *h){
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
 // struct hdr_gpsr_data *gdh = HDR_GPSR_DATA(p);

//  add by anzizhao 
	//if (cmh->ptype()!=PT_GPSR)
		//printf("packet type point 2:%d at %f\n",cmh->ptype(),GPSR_CURRENT); correct
  if (! isturnon )
  {
    Packet::free(p);
    return ;
  }
  //struct gpsr_neighbor neighbor;
  



  if(iph->saddr() == my_id_){


    if(cmh->num_forwards() == 0){ 
	  
 
      struct hdr_gpsr_data *gdh = HDR_GPSR_DATA(p);
      cmh->size() += IP_HDR_LEN + gdh->size();

      gdh->type_ = GPSRTYPE_DATA;
      gdh->mode_ = GPSR_MODE_GF;
      gdh->sx_ = (float)my_x_;
      gdh->sy_ = (float)my_y_;
      gdh->lx_ = (float)my_x_;
      gdh->ly_ = (float)my_y_;
	  gdh->seq_ = myseq_++;	
      double tempx, tempy;
      int hops;

      sink_list_->getLocbyID(iph->daddr(), tempx, tempy, hops);
      printf("recv() start\n");
      printf(" my id %d,seq_ = %d\n",my_id_, gdh->seq_);
      printf("start act node: %d x-y after function:%f %f \n",my_id_,tempx,tempy);


      if(tempx >= 0.0 && tempy >= 0.0){
	gdh->dx_ = (float)tempx;
	gdh->dy_ = (float)tempy;
      }
      else {
		  //if (cmh->ptype()!=PT_GPSR)
		//printf("packet type point 6:%d at %f\n",cmh->ptype(),GPSR_CURRENT); correct
	printf("node %d error test1(No sink) at %f\n",my_id_,GPSR_CURRENT);
	drop(p, "NoSink");
	return;
      }
      gdh->ts_ = (float)GPSR_CURRENT;
      cmh->ts_ = (float)GPSR_CURRENT;

      cmh->direction() = hdr_cmn::DOWN;
      cmh->addr_type() = NS_AF_INET;
      cmh->next_hop_ = IP_BROADCAST;
      cmh->last_hop_ = my_id_ ;
      gdh->lx_ = my_x_;
      gdh->ly_ = my_y_;
      gdh->ts_ = (float)GPSR_CURRENT;
      //printf("source id: %d broadcast start at %f\n",my_id_,GPSR_CURRENT);
 // target_->recv(p, 0);
      send(p, 0);

      return ;
    }          //   if(cmh->num_forwards() == 0)                          
    else if(cmh->num_forwards() > 0){ //routing loop
     
      if(cmh->ptype() != PT_GPSR)
      {
        //printf("===================================================================================\n");
       // printf("In recv(),iph->saddr() = %d and my id is %d\n",iph->saddr(),my_id_);
       // printf("GPSR_CURRENT; %f \n",GPSR_CURRENT );

       // printf("drop\n");
	//printf("packet type point 3:%d at %f\n",cmh->ptype(),GPSR_CURRENT); 
	//printf("node %d error test2 at %f\n",my_id_,GPSR_CURRENT);	  
	  drop(p, DROP_RTR_ROUTE_LOOP);
		}
      else{   //routing loop , cmh->ptype() == PT_GPSR  ,and iph->saddr() == my_id_ , and cmh->num_forwards() > 0
       

      //printf("GGGGG\n");
    struct hdr_gpsr *gh = HDR_GPSR(p);
    switch(gh->type_){
    case GPSRTYPE_HELLO:
      //printf("In recv(),enter recvHello\n");
      //printf("hello packet!\n");
      break;
    case GPSRTYPE_QUERY:
    //printf("In recv(),enter recvQuery()\n");
      //printf("query packet!\n");
      break;
    case GPSRTYPE_DATA:
    //printf("In recv(),enter recvQuery()\n");
      //printf("data packet!\n");
      break;
    default:
      //printf("default\n");
      //printf("Error with gf packet type.\n");
      break;
    }
  
        Packet::free(p);  
         //printf("Free the packet,and my id %d\n",my_id_);//, gdh->seq_
         }          
      return; 
    } //end  else if(cmh->num_forwards() > 0)
  } //end if(iph->saddr() == my_id_)
  







		//if (cmh->ptype()!=PT_GPSR)
		//printf("packet type point 1:%d at %f\n",cmh->ptype(),GPSR_CURRENT); a non_GPSR packet won't come here
 /* */
  if(cmh->ptype() == PT_GPSR){
    struct hdr_gpsr *gh = HDR_GPSR(p);
    switch(gh->type_){
    case GPSRTYPE_HELLO:
      //printf("In recv(),enter recvHello\n");
      recvHello(p);
      break;
    case GPSRTYPE_QUERY:
    //printf("In recv(),enter recvQuery()\n");
      recvQuery(p);
      break;
    default:
      //printf("Error with gf packet type.\n");
      exit(1);
    }
  } 
  else {
    struct hdr_gpsr_data *gdh = HDR_GPSR_DATA(p);
    //printf("===================================================================================\n");
    //printf("In recv(),it's a data packet and my id is %d,seq_ = %d.\n",my_id_,gdh->seq_);
    //printf("cmh->last_hop_ : %d\n", cmh->last_hop_);
    //printf("GPSR_CURRENT; %f \n",GPSR_CURRENT );


    iph->ttl_--;
    if(iph->ttl_ == 0){
#ifdef KEN
  //struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);
  FILE *fp = fopen("recv.tr", "a+"); 
         fprintf (fp, "time %.3f  my_id %d(%f,%f) ttl drop \n ",
                  GPSR_CURRENT, my_id_, my_x_, my_y_ );
         fclose(fp);
 #endif
	//printf("node %d error test3 at %f\n",my_id_,GPSR_CURRENT);
      drop(p, DROP_RTR_TTL);
      return;
    }
    //struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);
   // printf("forward test by %d at %f\n",my_id_,GPSR_CURRENT);
	//printf("and my location is %f %f\n",my_x_,my_y_);
	//printf("my speed: %f my angle: %f\n",my_speed_,my_angle_);
	//nblist_->NB_test(my_id_);
	//sink_list_->sink_test();
    //printf("In recv(),enter rt_resolve()and my id is %d.\n",my_id_);
    rt_resolve(p);

  }
  #ifdef KEN
  //struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);
  FILE *fp = fopen("recv.tr", "a+"); 
         fprintf (fp, "time %.3f  my_id %d(%f,%f) \n ",
                  GPSR_CURRENT, my_id_, my_x_, my_y_ );
         fclose(fp);
 #endif
}



void 
GPSRAgent::trace(char *fmt, ...){
  va_list ap;
  if(!tracetarget)
    return;
  va_start(ap, fmt);
  vsprintf(tracetarget->pt_->buffer(), fmt, ap);
  tracetarget->pt_->dump();
  va_end(ap);
}

int
GPSRAgent::command(int argc, const char*const* argv){
  if(argc==2){
    if(strcasecmp(argv[1], "getloc")==0){
      getLoc();
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "turnon")==0){
      turnon();
      return TCL_OK;
    }
    
    if(strcasecmp(argv[1], "turnoff")==0){
      turnoff();
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "startSink")==0){
      startSink();
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "neighborlist")==0){
      nblist_->dump();
      return TCL_OK;
    }
    if(strcasecmp(argv[1], "sinklist")==0){
      sink_list_->dump();
      return TCL_OK;
    }
  }


  if(argc==3){
    if(strcasecmp(argv[1], "startSink")==0){
      startSink(atof(argv[2]));
      return TCL_OK;
    }

    if(strcasecmp(argv[1], "addr")==0){
      my_id_ = Address::instance().str2addr(argv[2]);
      return TCL_OK;
    } 

    TclObject *obj;
    if ((obj = TclObject::lookup (argv[2])) == 0){
      fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__, argv[1],
	       argv[2]);
      return (TCL_ERROR);
    }
    if (strcasecmp (argv[1], "node") == 0) {
      node_ = (MobileNode*) obj;
      return (TCL_OK);
    }
    else if (strcasecmp (argv[1], "port-dmux") == 0) {
      port_dmux_ = (PortClassifier*) obj; //(NsObject *) obj;
      return (TCL_OK);
    } else if(strcasecmp(argv[1], "tracetarget")==0){
      tracetarget = (Trace *)obj;
      return TCL_OK;
    }
 //             if (strcasecmp(argv[1], "p2pSend") == 0) {
    //                    echo_p2p_send(atoi(argv[2]));
       //                 return (TCL_OK);
          //      }


  }// if argc == 3

  return (Agent::command(argc, argv));
}

void
GPSRAgent::addCarry(Packet *p)
{
	//printf("node %d enter addCarry function at %f\n",my_id_,GPSR_CURRENT);
	struct carry_buffer *temp = my_buffer_;

	temp = (struct carry_buffer*)malloc(sizeof(struct carry_buffer));
	temp->next_ = my_buffer_;
	temp->buffer = p;
	my_buffer_ = temp;
	carry_timer_.resched(carry_period_);
}

int
GPSRAgent::sendCarry()
{
	//printf("node %d enter sendCarry function at %f\n",my_id_,GPSR_CURRENT);
	struct carry_buffer *temp = my_buffer_;
	bool forward = false;
	if(temp==NULL)
		return 0;
	while(temp)
	{
		forward = get_target(temp->buffer);
		if(forward== true){
			forwardData(temp->buffer);
			removeCarry(temp->buffer);
			return 1;
		}
		else
		temp = temp->next_;
	}
	return 2;
	
}

void
GPSRAgent::removeCarry(Packet *target)
{
  //printf("node %d enter removeCarry function at %f\n",my_id_,GPSR_CURRENT);
  struct carry_buffer *temp;
  struct carry_buffer *p, *q;

  q = NULL;
  p = my_buffer_;
  while(p){
    temp = p;
    if(temp->buffer == target){
      p = temp->next_;
      if(q){
	q->next_ = p;
      }
      else {
	my_buffer_ = p;
      }
      free(temp);
      break;
    }

    q = p;
    p = p->next_;
  }
}

bool 
GPSRAgent::get_target(Packet *p)
{
 // printf("node %d enter get_target function at %f\n",my_id_,GPSR_CURRENT);
  //struct hdr_cmn *cmh = HDR_CMN(p);//包含packet type, timestamp, prev_hop, next_hop, last_hop
  struct hdr_ip *iph = HDR_IP(p);//包含src_id, dst_id, ttl counter
  struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);
  nblist_->myinfo(my_id_, my_x_, my_y_,my_speed_,my_angle_);
  
  double dx,dy;
  dx = gdh->dx_;
  dy = gdh->dy_;
  nsaddr_t nexthop = -1;
  
#ifdef UPDATE_DATAPACKET		  
		nblist_->NB_test(my_id_);
		//printf("node %d 's sinklist\n",my_id_);
		sink_list_->sink_test();
		double update_ts = sink_list_->getTimeStamp(iph->daddr());
		if((update_ts - gdh->ts_) > double(update_sink_loc_period_*5)){
			//printf("before setnewlocation x: %f y: %f\n",gdh->dx_,gdh->dy_);
			double tempdx = gdh->dx_;
			double tempdy = gdh->dy_;
			sink_list_->SetNewLocation(iph->daddr(),tempdx,tempdy);
			gdh->dx_ = tempdx;
			gdh->dy_ = tempdy;
			//printf("after setnewlocation x: %f y: %f\n",gdh->dx_,gdh->dy_);
			dx = gdh->dx_;
			dy = gdh->dy_;
		}
#endif

  nexthop = nblist_->gf_nexthop(dx, dy);
  if(nexthop == -1)
	return false;
  else
	return true;
}

/*GPSRAgent::GPSRBroadcast() : Agent(PT_GPSR){
 
  head_ = tail_ = NULL;
   
}

 GPSRAgent::~GPSRBroadcast( ){
 
  struct broadcast_buffer *temp = head_;
  while(temp){
    temp = temp->next_;
    free(head_);
    head_ = temp;
  }
}*/

void 
GPSRAgent::newBroadcastTimer(Packet *p,double waiting_time){
    struct  broadcast_buffer *temp = head_;
    struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p); 
    //printf("==============In newBroadcastTimer==============\n");
    //printf("My id is %d\n",my_id_);
 
    while ( temp ) {
      if ( temp->seq_ == gdh->seq_ ) {

       return ;
      
      }
      temp = temp->next_;
    
    }
  

    //if(temp==NULL){ //it is a new neighbor
    temp=( struct  broadcast_buffer*)malloc(sizeof( struct  broadcast_buffer));
    temp->packet = p->copy();
    struct hdr_cmn *cmh = HDR_CMN(temp->packet);
    struct hdr_ip *iph = HDR_IP(temp->packet);
    gdh=HDR_GPSR_DATA(temp->packet); 
    

//printf("temp->next_hop_: %d    cmh->next_hop_: %d\n",temp->next_hop_,cmh->next_hop_);
//printf("temp->last_hop_: %d    cmh->last_hop_: %d\n",temp->last_hop_,cmh->last_hop_);
//printf("temp->addr_type_: %d    cmh->addr_type_: %d\n",temp->addr_type_,cmh->addr_type_);
   temp->ts_= gdh->ts_+waiting_time;   
   temp->seq_ = gdh->seq_;  


  //printf("In newBroadcastTimer(),gdh->ts_ = %f \n",gdh->ts_);

  //printf("waiting_time= %f \n",waiting_time);
  //printf("gdh->ts_ = %f \n",gdh->ts_);  
  //printf("temp->ts_ = %f \n",temp->ts_);
  //printf("temp->seq_ = %d\n", temp->seq_);
  //printf("cmh->last_hop_ = %d\n",cmh->last_hop_ );


  temp->next_  = NULL;
  temp->prev_  = NULL;

  if(tail_ == NULL){ //the list now is empty
    head_ = tail_  = temp;
    //printf(" if(tail_ == NULL)\nhead_ = tail_  = temp;\n");

  }
  else { //now the neighbors list is not empty
    //printf(" if(tail_ != NULL)\n");

    tail_->next_ = temp;
    temp->prev_ = tail_;
    tail_ = temp;      
    temp->next_ = NULL;
  }

  //del_Broadcast_seq(temp->seq_);  //for test

}


void 
GPSRAgent::find_BroadcastTimer(  float timestamp_ ){
//  struct gpsr_neighbor *temp = getnb(nid);
  struct  broadcast_buffer *temp = head_;
  printf("==============In find_BroadcastTimer()==============\n");
  printf("GPSR_CURRENT:  %.9f\n",GPSR_CURRENT);
  printf("timestamp_: %f\n",timestamp_ );
  printf("My id is %d\n\n",my_id_);

  while ( temp != NULL ) {
    double temp_time = fabs(temp->ts_ - timestamp_) ;
    //printf("temp->ts_ :%.9f\n", temp->ts_);
    //printf("timestamp_ :%.9f\n", timestamp_);
    //printf("temp_time :%.9f\n", temp_time);

    if (  temp_time <= 0.000001   ) {

  //printf(" temp->ts_ == ts_\n");
  Packet  *p = temp->packet;
  struct hdr_cmn *cmh = HDR_CMN(p);
  struct hdr_ip *iph = HDR_IP(p);
  struct hdr_gpsr_data *gdh=HDR_GPSR_DATA(p);
  

  //cmh->direction() = hdr_cmn::DOWN;
  //cmh->addr_type() = NS_AF_INET;
   //printf("temp->ts_ = %f \n",temp->ts_);

  //printf("temp->ts_ = %f ,gdh->ts_ = %f\n",temp->ts_,gdh->ts_);
  //printf("temp->seq_ = %d\n", temp->seq_);
  printf("cmh->last_hop_ = %d\n", cmh->last_hop_);



 
    cmh->direction() = hdr_cmn::DOWN;
    cmh->addr_type() = NS_AF_INET;
    cmh->next_hop_ = IP_BROADCAST;


 // target_->recv(p, 0);
  send(p, 0);
  del_Broadcast_seq(temp->seq_);
  //printf("cmh->type() = %d\n",cmh->ptype());
  //printf("iph->saddr() = %d\n",iph->saddr());
  //printf("iph->daddr() = %d\n",iph->daddr());
  //printf("After send() and del_Broadcast_seq()\n");

      break ;
      
    }

    temp = temp->next_;
  //printf("6\n");
  }

  
}
  

void 
GPSRAgent::del_Broadcast_seq( u_int32_t   seq_){
//  struct gpsr_neighbor *temp = getnb(nid);
  struct  broadcast_buffer *temp = head_;
  struct  broadcast_buffer *q;
 //printf("==============In del_Broadcast_seq()==============\n");
 
  
  while(temp != NULL){
    //printf("in while(temp)\n");

      if(temp->seq_ == seq_){      
        //printf("seq_ = %d\n",seq_ );

          if(temp->prev_ == NULL){//表示這是第一個
                   //printf(" if(temp->prev_ == NULL)\n" );
                head_ = temp->next_;
                //nb->next_ = NULL;

                if(head_ == NULL) {
                  //printf(" if(head_ == NULL)\n" );

                  tail_ = NULL;
                  }
                else {
                  head_->prev_ = NULL;
                  }

                free(temp);
                break;
          } 
        else {
            //printf("2222\n");
            temp->prev_->next_ = temp->next_;
            

            if(temp->prev_->next_ == NULL){
              tail_ = temp->prev_;
             // printf("3333\n");
            }
            else {        
            //  printf("4444\n");      
              (temp->prev_->next_)->prev_ = temp->prev_;
            }
            temp->prev_ = NULL;
          //  printf("555\n");
            free(temp);
            break;
        }     
    
    
    }
   // printf("1\n");
    temp = temp->next_;

  }
}
  
bool
GPSRAgent::check_Broadcast_seq(u_int32_t   seq_){

//  struct gpsr_neighbor *temp = getnb(nid);
  //struct  broadcast_buffer *temp = head_;
 struct  broadcast_buffer *temp = head_;
 //printf("==============In check_Broadcast_seq()==============\n");
 
  
  while(temp != NULL){
   // printf("in while(temp)\n");

      if(temp->seq_ == seq_){      
       // printf("seq_ = %d\n",seq_ );
      //  printf("In check_Broadcast_seq(),return true\n");
        return true;          
        } 
    temp = temp->next_;

  }
 // printf("Not find seq_ %d\n",seq_);
  return false ;
}




