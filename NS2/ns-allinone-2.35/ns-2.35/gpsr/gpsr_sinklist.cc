/* -*- Mode:C++; c-basic-offset: 2; tab-width:2; indent-tabs-width:t -*- 
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

/* gpsr_sinklist.cc : the definition file for the sink list class
 *
 */

#include "gpsr_sinklist.h"


Sinks::Sinks(){
  sinklist_ = NULL;
}
Sinks::~Sinks()
{
 
  struct sink_entry *temp = sinklist_;
  struct sink_entry *del = NULL;
  while ( temp )
  {
    del = temp;
    temp = temp->next_;
    free(del);
      
  }
  sinklist_ = NULL;
  

}

bool
Sinks::new_sink(nsaddr_t id, double x, double y, 
		nsaddr_t lasthop, int hops, int seqno, double speed, double path){
  struct sink_entry *temp = sinklist_;
  while(temp){
    if(temp->id_ == id){
      if(temp->x_ == x && temp->y_ == y && 
         ((seqno <= temp->seqno_) || (temp->hops_ <= hops)))
        return false;
      else {
        temp->x_ = x;
        temp->y_ = y;
        temp->lasthop_ = lasthop;
        temp->hops_ = hops;
        temp->seqno_ = seqno;
        temp->speed_ = speed;
        temp->angle_ = path;
        temp->ts_ = GPSR_CURRENT;
      //  printf("new_sink function by ID(node:%d) success proof\n",temp->id_);
        return true;
      }
    }
    temp = temp->next_;
  }
  printf("new_sink function by ID new sink : %d at %f\n",id,GPSR_CURRENT);
  temp = (struct sink_entry*)malloc(sizeof(struct sink_entry));
  temp->next_ = sinklist_;
  temp->id_ = id;
  temp->x_ = x;
  temp->y_ = y;
  temp->lasthop_ = lasthop;
  temp->hops_ = hops;
  temp->seqno_ = seqno;
  temp->speed_ = speed;
  temp->angle_ = path;
  temp->ts_ = GPSR_CURRENT;
  sinklist_ = temp;
  return true;
}


bool
Sinks::new_sink(struct sink_entry &entry){
  struct sink_entry *temp = sinklist_;
  while(temp){
	  if(entry.x_<0.0 || entry.y_<0.0)
		return false;
//	  printf("new_sink function by sink_entry temp ID : %d\n",temp->id_);
    if(temp->id_ == entry.id_){
		
      if(temp->x_ == entry.x_ && temp->y_ == entry.y_ && entry.seqno_ <= temp->seqno_) 
        return false;
      else {
		 struct sink_entry *next = temp->next_;

     //   printf("time stamp test at %f and time stamp is %f\n",GPSR_CURRENT,entry.ts_);
        temp->id_ = entry.id_ ;
        temp->x_ = entry.x_;
        temp->y_ = entry.y_;
        temp->lasthop_ = entry.lasthop_;
        temp->seqno_ = entry.seqno_;
        temp->speed_ = entry.speed_;
        temp->angle_ = entry.angle_;
        temp->ts_ = entry.ts_;
      //  printf("new_sink function by sink_entry success proof and ID : %d\n",temp->id_);
        temp->next_ = next;
        return true;
      }
    }
    temp = temp->next_;
  }
 // printf("new_sink function by sink_entry and a new sink id: %d\n",entry.id_);
  temp = (struct sink_entry*)malloc(sizeof(struct sink_entry));

  temp->id_ = entry.id_ ;
  temp->x_ = entry.x_;
  temp->y_ = entry.y_;
  temp->lasthop_ = entry.lasthop_;
  temp->seqno_ = entry.seqno_;
  temp->speed_ = entry.speed_;
  temp->angle_ = entry.angle_;
  temp->ts_ = entry.ts_;
        
  temp->next_ = sinklist_;
  sinklist_ = temp;
  return true;
}





void
Sinks::remove_sink(nsaddr_t id){
  struct sink_entry *temp;
  struct sink_entry *p, *q;

  q = NULL;
  p = sinklist_;
  while(p){
    temp = p;
    if(temp->id_ == id){
      p = temp->next_;
      if(q){
	q->next_ = p;
      }
      else {
	sinklist_ = p;
      }
      free(temp);
      break;
     // return true;
    }//end if

    q = p;
    p = p->next_;
  }
  //return false;
}

bool
Sinks::remove_timeout(){
  struct sink_entry *temp = sinklist_;
  struct sink_entry *pre ;
  //struct sink_entry *buffer = NULL;
  //bool change = false;
  if ( ! sinklist_ )
    return true;

  //  double delperiod = 1.5 * query_period_;
  // 删除头部过期的

 /* while ( ( now_time - temp->ts_ ) > delperiod )
  {
    struct sink_entry *tmp = temp;
    temp = temp->next_;
    free(tmp);
  }
  // 第一个不过期的
  sinklist_ = temp;
  pre = temp;
  temp = temp->next_;
  */
/*   while ( temp ) {
  printf("id: %d GPSR -timestamp = %f - %f = %f  and delperiod = %f\n",
  temp->id_,GPSR_CURRENT,temp->ts_,(GPSR_CURRENT-temp->ts_),delperiod );
    if(( GPSR_CURRENT - temp->ts_ ) > delperiod){
      temp = temp->next_;
      change = true;
      printf("use test \n");
    }
    else {
		  pre = temp;
		  pre->next_ = buffer;
		  buffer = pre;
		  temp = temp->next_;   
    }
    
  }
  if(change)
  sinklist_ = buffer;*/

  while ( temp ) {

  //printf("id: %d GPSR -timestamp = %f - %f = %f  and delperiod = %f\n",1
  //temp->id_,GPSR_CURRENT,temp->ts_,(GPSR_CURRENT-temp->ts_),delperiod );1
  //printf("location: %f %f\n",temp->x_,temp->y_);1
  //printf("speed: %f angle: %f\n",temp->speed_,temp->angle_);1
    if(( GPSR_CURRENT - temp->ts_ ) > delperiod){
     // pre->next_ = temp->next_;
      //free(temp);
      //temp = pre->next_;
      //printf("remove sink id %d at %f\n",temp->id_,GPSR_CURRENT);
      //printf("remove_sink test\n");
      remove_sink(temp->id_);//這個function可以使用,沒有更動到location的資訊
      temp = temp->next_;
    }
    else {
      pre = temp;
      temp = temp->next_;
      
    }
    
  }
  pre = sinklist_;
  while(pre)
  {
	 // printf("check remove_sink fuction affection.\n");1
	  //printf("id: %d location: %f %f\n",pre->id_,pre->x_,pre->y_);1
	  pre = pre->next_;
  }
 // printf("\n");1
  return true ;
  
}


void
Sinks::getLocbyID(nsaddr_t id, double &x, double &y, int &hops){
  struct sink_entry *temp = sinklist_;
  //if(temp!=NULL)
	//printf("before loop temp->id :%d \n",temp->id_);
//	printf("getLocbyID function test by node : %d\n",id);
  while(temp){
    // add by anzizhao
    // 超出一个周期query信息 不能使用
  //  printf("temp->id :%d in loop at %f and time stamp is %f\n",temp->id_,GPSR_CURRENT,temp->ts_);
    /*if (  (GPSR_CURRENT - temp->ts_) > usefulperiod )
    {
      temp = temp->next_;
      continue;
    }*/
    
    if(temp->id_ == id){
      x = temp->x_;
      y = temp->y_;
      hops = temp->hops_;
      printf("node: %d xy: %f %f in getLocbyID function\n",id,x,y);
      return;
    }
    temp = temp->next_;
  }
  x = 1.0;
  y = 1.0;
}


// modify by anzizhao 
void
Sinks::getLocbyID(nsaddr_t id, double &x, double &y, int &hops, u_int8_t &seqno, double &ts,
					double &speed, double &angle){
  struct sink_entry *temp = sinklist_;
  while(temp){
     // add by anzizhao
    // 超出一周期query 不能使用
  //  printf("temp->id:%d at %f \n",temp->id_,GPSR_CURRENT);
   /* if (  (GPSR_CURRENT - temp->ts_) > usefulperiod )
    {
      temp = temp->next_;
      continue;
    }*/
    
    if(temp->id_ == id){
      x = temp->x_;
      y = temp->y_;
      hops = temp->hops_;
      seqno = temp->seqno_;
      speed = temp->speed_;
      angle = temp->angle_;
      ts = temp->ts_;
      return;
    }
    temp = temp->next_;
  }
  x = -1.0;
  y = -1.0;
}

void 
Sinks::dump(){
  struct sink_entry *temp = sinklist_;
  FILE *fp = fopen("sinklist.tr", "a+");
  while(temp){
    fprintf(fp, "%f\t%d\t%f\t%f\t%d\t%d\n", 
            temp->ts_,temp->id_, temp->x_, temp->y_, 
            temp->lasthop_,  temp->seqno_);
    temp = temp->next_;
  }
  fclose(fp);
}



// modify by anzizhao 
void Sinks::clear()
{
  struct sink_entry *temp = sinklist_;
  struct sink_entry *del = NULL;
  while ( temp )
  {
    del = temp;
    temp = temp->next_;
    free(del);
      
  }
  sinklist_ = NULL;
    

}

// modify by anizzhao
bool  Sinks::updateLocbyID(nsaddr_t id, double x, double y, int hops=0,double ts = 0.0 )//根本沒有被使用
 {
   struct sink_entry *temp = sinklist_;
   while(temp){
      if(temp->id_ == id){
      
        if ( temp->x_ == x && temp->y_ == y  && hops == 0)
          return false;
        
          
        temp->x_ = x;
        temp->y_ = y;
        temp->ts_ = GPSR_CURRENT;
        if ( hops )
          temp->hops_ = hops ;

        if ( ts > -1e-6 && ts < 1e-6 ) // == 0
          temp->ts_ = GPSR_CURRENT;
        else
          temp->ts_ = ts;
        
        return true;
       
      }
      temp = temp->next_;
      
   }
   return false;
 }
void Sinks::sink_test()
{
	struct sink_entry *temp =sinklist_;
	while(temp)
	{
		printf("in recv(),sink_test function , sink id :%d location: %f %f  situration: %f %f time stamp :%f\n"
		,temp->id_,temp->x_,temp->y_,temp->speed_,temp->angle_,temp->ts_);
		temp = temp->next_;
	}
}

bool  Sinks::updateLocbyID(struct sink_entry &entry )
{
   struct sink_entry *temp = sinklist_;
   while(temp){
     if(temp->id_ == entry.id_){
       
       temp->x_ = entry.x_;
       temp->y_ = entry.y_;
       temp->speed_ = entry.speed_;
       temp->angle_ = entry.angle_;
       temp->ts_ = GPSR_CURRENT;
       //    temp->hops_ = entry.hops_;
       //temp->ts_ = entry.ts_;
       if ( entry.seqno_ != 0 )
         temp->seqno_  = entry.seqno_;
       temp->lasthop_ = entry.lasthop_;
        
        return true;
       
      }
      temp = temp->next_;
      
   }
   return false;
 }

bool
Sinks::update_sink_loc(nsaddr_t id,double x,double y)
{
  struct sink_entry *temp = sinklist_;
  while(temp){
    if(temp->id_ == id){
temp->x_ = x;
temp->y_ = y;
temp->ts_ = GPSR_CURRENT;
	//	printf("%d sink's stamp time now is : %f\n",temp->id_,temp->ts_);
return true;
    }
    temp = temp->next_;
  }
  return false;
}

double 
Sinks::getTimeStamp(nsaddr_t id)
{
	struct sink_entry *temp = sinklist_;
	while(temp){
    if(temp->id_ == id)
		return temp->ts_;
	temp = temp->next_;
	}
	return 0.0;
}

void 
Sinks::SetNewLocation(nsaddr_t id, double &x, double &y)
{
	struct sink_entry *temp = sinklist_;
	while(temp){
		//printf("dst id: %d and temp id: %d\n",id,temp->id_);
		if(temp->id_ == id)
		{
			x = temp->x_;
			y = temp->y_;
			return;
		}
		temp = temp->next_;
	}
}
