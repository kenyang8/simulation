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

/* gpsr.h : The head file for the GPSR routing agent, defining the 
 *          routing agent, methods (behaviors) of the routing 
 *          
 * Note: the routing table (local neighborhood) information is kept 
 *       in another class gpsr_neighbor which is defined in 
 *       gpsr_neighbor{.h, .cc}. So the planarizing and next hop deciding
 *       is made there, not in this file
 *
 */

#ifndef GPSR_ROUTING_H_
#define GPSR_ROUTING_H_

#include "config.h"
#include "agent.h"
#include "ip.h"
#include "address.h"
//#include "scheduler.h"
#include "timer-handler.h"
#include "mobilenode.h"
#include "tools/random.h"
#include "packet.h"
#include "trace.h"
#include "classifier-port.h"
#include <math.h>
#include <cmu-trace.h>

#include "gpsr_packet.h"
#include "gpsr_neighbor.h"
#include "gpsr_sinklist.h"
#include "gpsr-seqtable.h"


class GPSRAgent;

class GPSRBroadcast;

class Echo_Timeout;

struct broadcast_buffer{
	


       Packet *packet ;
	u_int32_t   seq_; 
  	float ts_;      //the originating time stamp



	struct broadcast_buffer *next_;
	struct broadcast_buffer *prev_;
};


struct carry_buffer{
  Packet *buffer;
  struct carry_buffer *next_;
};
/*
 * Hello timer which is used to fire the hello message periodically
 */
class GPSRHelloTimer : public TimerHandler {
public:
  GPSRHelloTimer(GPSRAgent *a) : TimerHandler() {a_=a;}
protected:
  virtual void expire(Event *e);
  GPSRAgent *a_;
};

class GPSRCarryTimer : public TimerHandler {
public:
  GPSRCarryTimer(GPSRAgent *a) : TimerHandler() {a_=a;}
protected:
  virtual void expire(Event *e);
  GPSRAgent *a_;
  
};
/*
 * The Query Timer which is used by the data sink to fire the 
 * data query. It is not a part of the design of the GPSR routing.
 * Since the information of the data sink mostly is not able to be 
 * obtained directly (mostly, by DHT: distributed hash table), I 
 * just let the data sink to trigger the routing, like a common 
 * On-Demond routing.
 * 
 */
class GPSRQueryTimer : public TimerHandler {
public:
  GPSRQueryTimer(GPSRAgent *a) : TimerHandler() {a_=a;}
protected:
  virtual void expire(Event *e);
  GPSRAgent *a_;
};

class GPSRUpdateSinkLocTimer : public TimerHandler {
public:
GPSRUpdateSinkLocTimer(GPSRAgent *a) : TimerHandler() {a_=a;}
protected:
virtual void expire(Event *e);
GPSRAgent *a_;
};

class GPSRAgent : public Agent {
private:


  Echo_Timeout* EchoT;
  void echo_p2p_send(int dest);
  Event *p2pSendEvent;
  struct broadcast_buffer *head_=NULL;
  struct broadcast_buffer *tail_=NULL;

  

  friend class GPSRHelloTimer;
  friend class GPSRQueryTimer;
  friend class GPSRUpdateSinkLocTimer;
  friend class GPSRCarryTimer;
  friend class Echo_Timeout;
  MobileNode *node_;             //the attached mobile node
  PortClassifier *port_dmux_;    //for the higher layer app de-multiplexing
  
  nsaddr_t my_id_;               //node id (address), which is NOT necessary
  double my_x_;                  //node location info, fatal info
  double my_y_;                  //     obtained from the attached node
  double my_speed_;
  double my_angle_;
  double x_temp;
  double y_temp;
  double time_record;
  bool carry_and_forward;
  struct carry_buffer *my_buffer_;
  struct broadcast_buffer *GPSRPacketList_;
  //broadcast_buffer *pklist_;
  int carry_counter;
  // record status 
  bool isturnon;
  
  Sinks *sink_list_;      //for multiple sinks

  GPSRNeighbors *nblist_; //neighbor list: routing table implemenation 
                          //               and planarizing implementation

  //GPSRBroadcast *GPSRPacketList_;
  int recv_counter_;           
  u_int8_t query_counter_;

  GPSRHelloTimer hello_timer_;
  GPSRQueryTimer query_timer_;
  GPSRUpdateSinkLocTimer update_sink_loc_timer_;
  GPSRCarryTimer carry_timer_;
  
  int planar_type_; //1=GG planarize, 0=RNG planarize

  double hello_period_;
  double query_period_;
  double carry_period_;
  double safe_distance_;
  double delperiod;
  double usefulperiod;
  double start_update_time_; 
  double update_sink_loc_period_;
  
  void turnon();              //set to be alive
  void turnoff();             //set to be dead
  void startSink();          
  void startSink(double);

  void GetLocation(double*, double*); //called at initial phase
  virtual void getLoc();

  void hellomsg();
  void getmyspeed();
  void getmyangle();
  //void replyloc(nsaddr_t,nsaddr_t,double,double);
  void query(nsaddr_t);

  void recvHello(Packet*);
  void recvQuery(Packet*);
  void addCarry(Packet*);
  int sendCarry();
  bool get_target(Packet*);
  void removeCarry(Packet*);
 // void recvUpdate(Packet*);
  void sinkRecv(Packet*);
  void forwardData(Packet*);

  RNG randSend_;

  /**
   * The below variables and functions are used for 
   * localization algorithms
   */
  double localized_x_; 
  double localized_y_;
  void dvhop();
  u_int32_t myseq_;

protected:
  Trace *tracetarget;              //for routing agent special trace
  void trace(char *fmt,...);       //   Not necessary 

  void hellotout();                //called by timer::expire(Event*)
  void querytout();
  void updatesinkloctout();
  void carryout();
  void rt_resolve(Packet *p);
  
  gpsr_RTable rtable_;
  
public:
  GPSRAgent();
 
  ~GPSRAgent();
  
  int command(int, const char*const*);
  void recv(Packet*, Handler*);         //inherited virtual function
  void Recv_callback(Event* e);
  void newBroadcastTimer(Packet*,double);
  void find_BroadcastTimer(  float );
  void del_Broadcast_seq( u_int32_t   );
  bool  check_Broadcast_seq(u_int32_t   );




};

class Echo_Timeout : public Handler {
  public:
    Echo_Timeout(GPSRAgent *a_) { a = a_; }
    virtual void handle(Event *e) { a->Recv_callback(e); }

  private:
    GPSRAgent *a;
};

/*class GPSRBroadcast : public Agent {

	private:
	struct broadcast_buffer *head_=NULL;
	struct broadcast_buffer *tail_=NULL;



	public:
		void newBroadcastTimer(Packet*,double);
		void find_BroadcastTimer(  float );
		void del_Broadcast_seq( u_int32_t   );
		bool  check_Broadcast_seq();
            GPSRBroadcast();
            ~GPSRBroadcast();

	
};*/

#endif
