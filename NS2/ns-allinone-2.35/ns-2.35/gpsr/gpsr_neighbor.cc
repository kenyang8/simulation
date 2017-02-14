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

#include "gpsr_neighbor.h"
#include <stdlib.h>

#define PI 3.141593

#define MAX(a, b) (a>=b?a:b)
#define MIN(a, b) (a>=b?b:a)

#define USABLETIME 5
#define DELETETIME 7
//#define KEN

GPSRNeighbors::GPSRNeighbors(){
  my_id_ = -1;
  my_x_ = 0.0;
  my_y_ = 0.0;
  my_speed_ = 0.0;
  my_angle_ = 0.0;
  head_ = tail_ = NULL;
  nbSize_ = 0;
  hello_period_ = 0.0;
  

}

GPSRNeighbors::~GPSRNeighbors(){
  struct gpsr_neighbor *temp = head_;
  while(temp){
    temp = temp->next_;
    free(head_);
    head_ = temp;
  }
}

double 
GPSRNeighbors::getdis(double ax, double ay, double bx, double by){
  double tempx = ax - bx;
  double tempy = ay - by;

  tempx = tempx * tempx;
  tempy = tempy * tempy;

  double result = sqrt(tempx + tempy);
  return result;
}

int
GPSRNeighbors::nbsize(){
  return nbSize_;
}

void
GPSRNeighbors::myinfo(nsaddr_t mid, double mx, double my, double speed, double angle){
  my_id_ = mid;
  my_x_ = mx;
  my_y_ = my;
  my_speed_ = speed;
  my_angle_ = angle;
}

struct gpsr_neighbor*
GPSRNeighbors::getnb(nsaddr_t nid){
     struct gpsr_neighbor *temp = head_;
     while(temp){
          if(temp->id_ == nid){
               // modify by anzizhao 
               // if((GPSR_CURRENT - temp->ts_) < DEFAULT_GPSR_TIMEOUT)
               //  return temp;
               // else {
               //      delnb(temp); //if this entry expire, delete it and return NULL
               //      return NULL;
               // }
               if ((GPSR_CURRENT - temp->ts_) < USABLETIME*hello_period_ )
                    return temp;
               else if ((GPSR_CURRENT - temp->ts_) < DELETETIME*hello_period_ )
                    return NULL;
               else
               {
                    delnb(temp); //if this entry expire, delete it and return NULL
                    return NULL;
               }
        
          }
          temp = temp->next_;
     }
     return NULL;
}

bool 
GPSRNeighbors::getnb(nsaddr_t nid, struct gpsr_neighbor &neighbor ){
  struct gpsr_neighbor *temp = getnb(nid);
  if ( temp == NULL )
  {
    
    return false;
    
  }

  neighbor = *temp;
  return true;
  
  
}
void GPSRNeighbors::NB_test(nsaddr_t id)
{
	printf("In recv(), NB_test fuction by node %d at %f\n",id,GPSR_CURRENT);
	struct gpsr_neighbor *temp = head_;
	while ( temp )
	{
	  printf("id: %d location: %f %f situration: %f %f time_stamp: %f\n"
	  ,temp->id_,temp->x_,temp->y_,temp->speed_,temp->angle_,temp->ts_);
      temp = temp->next_;
    }
    printf("\n");
}
void 
GPSRNeighbors::newNB(nsaddr_t nid, double nx, double ny, double nspeed, double nangle){

//  struct gpsr_neighbor *temp = getnb(nid);
  struct gpsr_neighbor *temp = head_;
  while ( temp ) {
    if ( temp->id_ == nid ) {
      temp->ts_ = GPSR_CURRENT;
      temp->x_ = nx; //the updating of location is allowed
      temp->y_ = ny;
      temp->speed_ = nspeed;
      temp->angle_ = nangle;
      return ;
      
    }
    temp = temp->next_;
    
  }
  
  //if(temp==NULL){ //it is a new neighbor
    temp=(struct gpsr_neighbor*)malloc(sizeof(struct gpsr_neighbor));
    temp->id_ = nid;
    temp->x_ = nx;
    temp->y_ = ny;
    temp->speed_ = nspeed;
    temp->angle_ = nangle;
    temp->ts_ = GPSR_CURRENT;
    temp->next_ = temp->prev_ = NULL;

    if(tail_ == NULL){ //the list now is empty
      head_ = tail_  = temp;
      nbSize_ = 1;
    }
    else { //now the neighbors list is not empty
      tail_->next_ = temp;
      temp->prev_ = tail_;
      tail_ = temp;
      nbSize_++;
    }
  //}

}

void
GPSRNeighbors::delnb(nsaddr_t nid){
  struct gpsr_neighbor *temp = getnb(nid);
  if(temp==NULL) return;
  else delnb(temp);
}

void
GPSRNeighbors::delnb(struct gpsr_neighbor *nb){
  struct gpsr_neighbor *preffix = nb->prev_;
  //printf("NB_list before delnb node %d function:\n",nb->id_);
	//NB_test(my_id_);
  if(preffix == NULL){//表示這是第一個
    head_ = nb->next_;
    //nb->next_ = NULL;

    if(head_ == NULL) 
      tail_ = NULL;
    else head_->prev_ = NULL;

    free(nb);
  }
  else {
    preffix->next_ = nb->next_;
    nb->prev_ = NULL;
    if(preffix->next_ == NULL)
      tail_ = preffix;
    else (preffix->next_)->prev_ = preffix;
    free(nb);
  }
  //printf("NB_list after delnb function:\n");
  //NB_test(my_id_);
  nbSize_--;
  
}

void
GPSRNeighbors::delalltimeout(){
  struct gpsr_neighbor *temp = head_;
  struct gpsr_neighbor *dd;
  while(temp){
    if((GPSR_CURRENT - temp->ts_) >= DELETETIME ){
      dd = temp;
      temp = temp->next_;
      delnb(dd);
    }
    else temp = temp->next_;
  }
  
}


nsaddr_t
GPSRNeighbors::gf_nexthop(double dx, double dy){
  struct gpsr_neighbor *temp = head_;
  //initializing the minimal distance as my distance to sink
  double mindis =getdis(my_x_, my_y_, dx, dy); //自己與dst的距離差距
  printf("node %d at %f enter gf_nexthop debug: mydistance is: %f\n",my_id_,GPSR_CURRENT,mindis);
  nsaddr_t nexthop = -1; //the nexthop result 
  
  while(temp){
    if ( ! InSafetyZone(temp->x_, temp->y_) )//temp xy與自己的xy取距離,如果小於safe_distance回傳true,不然回傳flase
    {//如果超過可傳輸範圍就換下一個neighbor,這個節點舊部選擇了
      double test = getdis(temp->x_,temp->y_,dx,dy);
      
      printf("nb %d is too far and distance is %f\n",temp->id_,test);
      temp = temp->next_;
      continue;
    }

    if ((GPSR_CURRENT - temp->ts_) < USABLETIME*hello_period_ )
    {
      double tempdis = getdis(temp->x_, temp->y_, dx, dy);
      printf("nb %d distance is: %f\n",temp->id_,tempdis);
      //printf("tempx: %f tempy: %f dx: %f dy: %f\n",temp->x_,temp->y_,dx,dy);
      if(tempdis < mindis){
        mindis = tempdis;
        nexthop = temp->id_;
      }
    }
    else if ((GPSR_CURRENT - temp->ts_) > DELETETIME*hello_period_ ){
      //double test1 = getdis(temp->x_,temp->y_,dx,dy);
     // printf("nb_list trying to delete %d and ts is %f\n",temp->id_,temp->ts_);
      delnb(temp);
      //temp = temp->next_;
      //continue;
    }
    temp = temp->next_;
  }
  printf("nexthop value before leaving gf_nexthop function: %d\n",nexthop);
  return nexthop;
}

nsaddr_t
GPSRNeighbors::gf_nexthop1(double dx, double dy,nsaddr_t id,double lx, double ly){
  struct gpsr_neighbor *temp = head_;
  //initializing the minimal distance as my distance to sink
  double mindis =getdis(my_x_, my_y_, dx, dy); //自己與dst的距離差距
  printf("node %d at %f enter gf_nexthop debug: mydistance is: %f\n",my_id_,GPSR_CURRENT,mindis);
  nsaddr_t nexthop = -1; //the nexthop result 
  
  double tempx = dx - lx;
  double tempy = dy - ly;
  double temp_angle = atan2(tempy,tempx) * 180 / PI;

  double tempx1 = my_x_ - lx;
  double tempy1 = my_x_ - ly;
  double temp_angle1 = atan2(tempy1,tempx1) * 180 / PI;
  

  if((temp_angle1<temp_angle+120)&&(temp_angle1>temp_angle-120))
  	nexthop = 1;

  else 
  	nexthop = 0;

#ifdef KEN
    
  FILE *fp = fopen("gf_nexthop1.tr", "a+"); 
         fprintf (fp, "time %.3f,angle:%f,angle1:%f  ,dst(%f,%f) my %d(%f,%f) lx: %f ly: %f nexthop %d\n ",
                  GPSR_CURRENT, temp_angle,temp_angle1, dx,
                  dy, id, my_x_, my_y_ ,lx,ly, nexthop );
         fclose(fp);
#endif
  	
  printf("nexthop value before leaving gf_nexthop function: %d\n",nexthop);
  return nexthop;
}

double
GPSRNeighbors::gf_nexthop2(double dx, double dy,nsaddr_t id,double lx, double ly,double my_speed_,double my_angle_ ){       // Waiting Time  calculate
  struct gpsr_neighbor *temp = head_;
  //initializing the minimal distance as my distance to sink
  double mindis =getdis(my_x_, my_y_, dx, dy); //自己與dst的距離差距
  printf("node %d at %f enter gf_nexthop2 debug: mydistance with destination is: %f\n",my_id_,GPSR_CURRENT,mindis);
  nsaddr_t nexthop = -1; //the nexthop result 
  double transmission_range = 200.0;
  double dis_last_hop =getdis(my_x_, my_y_, lx, ly); 
  double dis_dst =getdis(my_x_, my_y_, dx, dy);
  double dis_last_hop_nb =getdis(my_x_, my_y_, lx, ly);
  double max_dis =0.0;
  double nb_counter = 0.0;
  double nb_dis;



while(temp){      // calculate the max distance  and neighbor number .
    if ( ! InSafetyZone(temp->x_, temp->y_) )//temp xy與自己的xy取距離,如果小於safe_distance回傳true,不然回傳flase
    {//如果超過可傳輸範圍就換下一個neighbor,這個節點舊部選擇了
      double test = getdis(temp->x_,temp->y_,my_x_,my_y_);
      
      printf("nb %d is too far and distance is %f\n",temp->id_,test);
      temp = temp->next_;
      continue;
    }

    if ((GPSR_CURRENT - temp->ts_) < USABLETIME*hello_period_ )
    {
      double nb_dis = getdis(temp->x_, temp->y_, lx, ly); //neighbor's distance with destination
      //printf("nb %d distance is: %f \n ",temp->id_,nb_dis);
      //printf("tempx: %f tempy: %f dx: %f dy: %f\n",temp->x_,temp->y_,dx,dy);
       if( (nb_dis -dis_last_hop_nb)>30){
        //printf("nb_counter++,dis_last_hop_nb : %f,nb_dis: %f\n",dis_last_hop_nb,nb_dis );
        nb_counter++;
      }

       double tempdis = getdis(temp->x_,temp->y_,my_x_,my_y_);

      if((max_dis < tempdis) && (tempdis<=249)  ){
          max_dis = tempdis;
          nexthop = temp->id_;
        //nb_dis = getdis(temp->x_, temp->y_, my_x_, my_y_);

      }
    }
    else if ((GPSR_CURRENT - temp->ts_) > DELETETIME*hello_period_ ){
      //double test1 = getdis(temp->x_,temp->y_,dx,dy);
     // printf("nb_list trying to delete %d and ts is %f\n",temp->id_,temp->ts_);
      delnb(temp);
      //temp = temp->next_;
      //continue;
    }
    temp = temp->next_;
  }

double waiting_time;

if(nb_counter==0){
  waiting_time = -1;
  return waiting_time ;
     }
      else{


          double radian = my_angle_ * PI/180.0;
          double xt = my_x_ + (my_speed_*cos(radian));
          double yt = my_y_ + (my_speed_*sin(radian));
          double dt =getdis(xt, yt, dx, dy);
          double threshold = 3.0;

          printf("\n===============In gf_nexthop2===========================\n ");
          printf("radian: %.2f,  xt: %.2f,  yt: %.2f,  dt: %.2f,  my_speed_: %.2f\n",radian,xt,yt,dt ,my_speed_);
          printf("dis_last_hop: %.2f,  dis_dst: %.2f,  dis_last_hop_nb: %.2f\n",dis_last_hop,dis_dst,dis_last_hop_nb);
          printf("max_dis: %.2f,mindis: %.2f\n",max_dis,mindis );
          printf("nb_dis: %.2f\n",nb_dis );
          printf("cos(radian) :%f ,sin(radian): %f\n",cos(radian),sin(radian) );
          printf("my_x_: %.2f , my_y_: %.2f\n",my_x_ ,my_y_);
          printf("nb_counter: %f\n", nb_counter);
          double x = (threshold/(1+nb_counter));

          waiting_time = 0.03*(1-(dis_last_hop+max_dis)/(2*transmission_range))*(dt/dis_dst)*(threshold/(1+nb_counter));

          printf("waiting_time value before leaving gf_nexthop2 function: %f\n",waiting_time);
          //printf("=========================================================\n ");            
          return waiting_time;
        }
}

double 
GPSRNeighbors::gf_nexthop3(double lx, double ly){
  struct gpsr_neighbor *temp = head_;
  //initializing the minimal distance as my distance to sink
  double dis_lasthop =getdis(my_x_, my_y_, lx, ly); //自己與last hop的距離差距  
 // nsaddr_t nexthop = -1; //the nexthop result 
  double tempx = lx - my_x_;
  double tempy = ly - my_y_;
  double temp_angle = atan2(tempy,tempx) * 180 / PI;
  double range = 200.0 ;
  double default_delay = 0.01 * (1-(dis_lasthop/range));
  double waiting_time;
  //printf("\n===============In gf_nexthop3===========================\n ");
if (dis_lasthop <= 200){
  if ((temp_angle >= 67.5 && temp_angle <= 112.5 )||(temp_angle >= 157.5 && temp_angle <= (-157.5) )||(temp_angle >= -112.5 && temp_angle <= -67.5 )
    ||(temp_angle >= -22.5&& temp_angle <= 22.5) ){

    //printf("In the direaction.\n"); (temp_angle >= 67.5 && temp_angle <= 112.5 )||(temp_angle >= 157.5 && temp_angle <= (-157.5) )||(temp_angle >= -112.5 && temp_angle <= -67.5 )
    //||(temp_angle >= -22.5&& temp_angle <= 22.5)
    waiting_time = default_delay + 0.01*(rand() / RAND_MAX) ;
  }
  else {
    //printf("Not in the direaction.\n");   
    waiting_time = default_delay + (0.03*(rand() / RAND_MAX))+0.02 ;
  }
}
else
     waiting_time = -1;



//[down, up] 含上界
//rand_num = <up-down> * rand() / RAND_MAX + <down>;

  //printf("node %d at %f enter gf_nexthop3 debug: mydistance with last hop is: %f, and temp_angle = %f\n",my_id_,GPSR_CURRENT,dis_lasthop,temp_angle);
  //printf("default_delay: %f\n",default_delay );
  //printf("dis_lasthop: %f\n",dis_lasthop );
  //printf("temp_angle: %f\n",temp_angle );
  //printf("waiting_time value before leaving gf_nexthop3 function: %f\n",waiting_time);
  return waiting_time;
}



struct gpsr_neighbor *
GPSRNeighbors::gg_planarize(){
    struct gpsr_neighbor *temp, *result, *index;
    index = head_;
    result = NULL;
    
    while(index){

      if ( ! InSafetyZone(index->x_, index->y_) )
      {
        index = index->next_;
        continue;
      }
      if ( (GPSR_CURRENT - index->ts_) > USABLETIME*hello_period_ )
      {
        index = index->next_;
        continue;
      }
         
      double midpx = my_x_ + (index->x_ - my_x_)/2.0;
      double midpy = my_y_ + (index->y_ - my_y_)/2.0;
      double mdis = getdis(my_x_, my_y_, midpx, midpy);
      
      temp = head_;
      while(temp){
        if ( ! InSafetyZone(temp->x_, temp->y_) )
        {
          temp = temp->next_;
          continue;
        }
        
        if ( (GPSR_CURRENT - temp->ts_) > USABLETIME*hello_period_ )
        {
          temp = temp->next_;
          continue;
        }

        if(temp->id_ != index->id_){
          double tempdis = getdis(midpx, midpy, temp->x_, temp->y_);
          if(tempdis < mdis) break;
        }
        temp=temp->next_;
      }
      
      if(temp==NULL){
        temp = (struct gpsr_neighbor*)malloc(sizeof(struct gpsr_neighbor));
        temp->id_ = index->id_;
        temp->x_ = index->x_;
        temp->y_ = index->y_;
        //add by anzizhao
        temp->ts_ = index->ts_;
        
        temp->next_ = result;//result == NULL at first time
        temp->prev_ = NULL;
        if(result) 
			result->prev_ = temp;
        result = temp;
      }
      
      index=index->next_;
    }
  
    return result;

}


struct gpsr_neighbor *
GPSRNeighbors::rng_planarize(){
    struct gpsr_neighbor *temp, *result, *index;
    index = head_;
    result = NULL;
    
    while(index){

      if ( ! InSafetyZone(index->x_, index->y_) )
      {
        index = index->next_;
        continue;
      }
      if ( (GPSR_CURRENT - index->ts_) > USABLETIME*hello_period_ )
      {
        index = index->next_;
        continue;
      }
         
      double mdis = getdis(my_x_, my_y_, index->x_, index->y_);
      
      temp = head_;
      while(temp){
        
        if ( ! InSafetyZone(temp->x_, temp->y_) )
        {
          temp = temp->next_;
          continue;
        }
        
        if ( (GPSR_CURRENT - temp->ts_) > DELETETIME*hello_period_ )
        {
          temp = temp->next_;
          continue;
        }
        if(temp->id_ != index->id_){
          double tempdis1 = getdis(my_x_, my_y_, temp->x_, temp->y_);
          double tempdis2 = getdis(index->x_, index->y_, temp->x_, temp->y_);
          if(tempdis1 < mdis && tempdis2 < mdis) break;
        }
        temp=temp->next_;
      }
      
      if(temp==NULL){
        temp = (struct gpsr_neighbor*)malloc(sizeof(struct gpsr_neighbor));
        temp->id_ = index->id_;
        temp->x_ = index->x_;
        temp->y_ = index->y_;
        temp->ts_ = index->ts_;
        
        temp->next_ = result;
        temp->prev_ = NULL;
        if(result) result->prev_ = temp;
        result = temp;
      }
      
      index=index->next_;
    }
  
    return result;

}


double
GPSRNeighbors::angle(double x1, double y1, double x2, double y2){
  double line_len = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

  double sin_theta, cos_theta;
  double theta;

  if(line_len == 0.0){
    printf("2 nodes are the same\n");
    return -1.0;
  }

  sin_theta = (y2-y1)/line_len;
  cos_theta = (x2-x1)/line_len;

  theta = acos(cos_theta);
  
  if(sin_theta<0){
    theta = 2*PI - theta;
  }

  return theta;
}


/* To check the line from me to theother, and the line from source
 * and destination is intersecting each other or not
 * Note: 2 line segments intersects each other if they have a common 
 *       point, BUT here, if the common point is the end point, 
 *       we don't count it.
 */
//  
int
GPSRNeighbors::intersect(nsaddr_t theother, double sx, double sy,
			 double dx, double dy){
  //line 1 (x1,y1)--(x2,y2) is the segment
  //line 2 (x3,y3)--(x4,y4) is the xD

  if ( theother == -1 )
    return 0;
  struct gpsr_neighbor *other = getnb(theother);

  if(other==NULL){

    printf("Wrong the other node\n");
    printf ( "%d is not  %d node neighbor in %.4f\n", theother, my_id_, GPSR_CURRENT);
    
    dump ();
    
    exit(1);
  }
  
  double x1 = my_x_; 
  double y1 = my_y_;
  double x2 = other->x_;
  double y2 = other->y_;
  double x3 = sx;
  double y3 = sy;
  double x4 = dx;
  double y4 = dy;

  // me --> theother 向量 (a1 b1)
  double a1 = y2 - y1;
  double b1 = x1 - x2;
  double c1 = x2*y1 - x1*y2;

  // src -- > dst 向量 (a2 b2）
  double a2 = y4 - y3;
  double b2 = x3 - x4;
 
  double c2 = x4*y3 - x3*y4;

  double denom = a1*b2 - a2*b1;

  double x, y; //the result;

  if(denom == 0) {
    return 0; //parallel lines;
  }
  x = (b1*c2 - b2*c1)/denom;
  y = (a2*c1 - a1*c2)/denom;

  if(x > MIN(x1, x2) && x < MAX(x1, x2) &&
     x > MIN(x3, x4) && x < MAX(x3, x4))
    return 1;       
  else return 0;
}


int
GPSRNeighbors::num_of_neighbors(struct gpsr_neighbor *nblist){
  struct gpsr_neighbor *temp = nblist;
  int counter = 0;
  while(temp){
    counter++;
    temp = temp->next_;
  }
  return counter;
}

void
GPSRNeighbors::free_neighbors(struct gpsr_neighbor *nblist){
  struct gpsr_neighbor *temp, *head;
  head = nblist;
  while(head){
    temp = head;
    head = head->next_;
    free(temp);
  }
}


nsaddr_t
GPSRNeighbors::peri_nexthop(int type_, nsaddr_t last,
			    double sx, double sy,double dx, double dy){//type_=1為預設值,last=-1為functiont傳入時給的,不然就是dst
  struct gpsr_neighbor *planar_neighbors, *temp;
  double alpha, minangle;
  nsaddr_t nexthop=-1;

  minangle = 2*PI;

  if(type_){//GG planarizing
    planar_neighbors =gg_planarize();
  }else {//RNG planarizing ,沒還進來過
    planar_neighbors = rng_planarize();
  }

  if(last>-1){
    struct gpsr_neighbor *lastnb = getnb(last);
    if(lastnb == NULL) {
      printf("Wrong last nb %d in %d node \n", last, my_id_);
      printf ("time %f\n", GPSR_CURRENT);    
      printf("my position: (%.2f , %.2f)\n\n", my_x_, my_y_);
      dump();     
      //exit (1);
      return -1;
   
    }
    else 
      alpha = angle(my_x_, my_y_, lastnb->x_, lastnb->y_);
    // struct gpsr_neighbor *lastnb = getnb(last);
    // if(lastnb == NULL) {
    //   alpha = angle(my_x_, my_y_, dx, dy); 
    // }
    // else 
    //   alpha = angle(my_x_, my_y_, lastnb->x_, lastnb->y_);
  }
  else 
    alpha = angle(my_x_, my_y_, dx, dy); 
  temp = planar_neighbors;
  
  while(temp){
    if(temp->id_ != last){
      double delta;
      delta = angle(my_x_, my_y_, temp->x_, temp->y_);
      delta = delta - alpha;
      if(delta < 0.0) {
	delta = 2*PI + delta;
      }
      
      if(delta < minangle){
	minangle = delta;
	nexthop = temp->id_;
      }
    }
    temp = temp->next_;
  }
  // // add by anzizhao
  // if ( nexthop == -1 )
  //      return nexthop ;

  // modify by anzizhao 
  // if(num_of_neighbors(planar_neighbors) > 1 &&
  //    intersect(nexthop, sx, sy, dx, dy)){
  //     free_neighbors(planar_neighbors);
  //     return peri_nexthop(type_, nexthop, sx, sy, dx, dy);
  // }
  if(num_of_neighbors(planar_neighbors) >= 1 ){
    // if ( my_id_ == 54) {
    //   FILE *fp = fopen(NB_TRACE_FILE, "a+"); 
    //   fprintf(fp, "nexthop %d\t%d\n",  my_id_, nexthop );
    //   dump(planar_neighbors);
    //   dump();
      
    // }
  
    free_neighbors(planar_neighbors);
    if (intersect(nexthop, sx, sy, dx, dy))
      return nexthop;
  }
 /* if (  my_id_ == 37 || my_id_ == 96 ) {
    dump();
    
    dump(planar_neighbors);
  }*/
  
  
  return nexthop;
}

void
GPSRNeighbors::dump(){
  delalltimeout();
  
  FILE *fp = fopen(NB_TRACE_FILE, "a+"); 

  struct gpsr_neighbor *temp = head_;
  fprintf(fp, "%f\t%d:\t\n", GPSR_CURRENT, my_id_);
  while(temp){
    fprintf(fp, "\t%f\t%d\t%f\t%f\n", temp->ts_, temp->id_,temp->x_, temp->y_);
    temp = temp->next_;
  }
  fprintf(fp,"\n");
  fclose(fp);
}

void
GPSRNeighbors::dump(struct gpsr_neighbor *head ){
  //delalltimeout();
  
  FILE *fp = fopen(NB_TRACE_FILE, "a+"); 

  struct gpsr_neighbor *temp = head;
  fprintf(fp, "%f\t%d:\t\n", GPSR_CURRENT, my_id_);
  while(temp){
    fprintf(fp, "\t%f\t%d\t%f\t%f\n", temp->ts_, temp->id_,temp->x_, temp->y_);
    temp = temp->next_;
  }
  fprintf(fp,"\n");
  fclose(fp);
}


void GPSRNeighbors::clear()
{
  struct gpsr_neighbor *temp = head_;
  struct gpsr_neighbor *dd;
  while(temp){
    dd = temp;
    temp = temp->next_;
    delnb(dd);
  }
  
  head_ = tail_ = NULL;
}
  
