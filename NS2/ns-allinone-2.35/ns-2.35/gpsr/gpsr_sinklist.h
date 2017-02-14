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

/* gpsr_sinklist.h : the sink table which maintain a list of data sinks 
 *                   it is used for multiple data sink, which is not 
 *                   a part of the design of GPSR.
 *                  
 *                   Not each node needs to maintain a list of the data sinks
 *                   Only the data source nodes may need this list
 */

#ifndef GPSR_SINK_LIST_H_
#define GPSR_SINK_LIST_H_

#include "config.h"
#include "gpsr_packet.h"


struct sink_entry{
  nsaddr_t id_;
  double x_;
  double y_;
  double speed_;
  double angle_;
  nsaddr_t lasthop_;
  int hops_;

  double ts_;
  int seqno_;
  
  struct sink_entry *next_;
};

class Sinks {
  struct sink_entry *sinklist_;
  double query_period_;
  double delperiod;
  double usefulperiod;
  
 public:
  Sinks();
  ~Sinks();
  
  bool new_sink(nsaddr_t, double, double, nsaddr_t, int, int, double, double);
  bool new_sink(struct sink_entry &);
  bool update_sink_loc(nsaddr_t, double, double);
  void remove_sink(nsaddr_t);
  // remove time out sink
  bool remove_timeout();
  
  void getLocbyID(nsaddr_t, double&, double&, int&);
  void dump();
  void sink_test();//make by myself
  double getTimeStamp(nsaddr_t);
  void SetNewLocation(nsaddr_t, double&, double&);
  void clear();
  bool updateLocbyID(nsaddr_t , double , double , int ,double );
  bool updateLocbyID(struct sink_entry &);
  void getLocbyID(nsaddr_t, double&, double&, int&, u_int8_t &, double&, double&, double&);

  void setPeriod (double query, double del , double useful )
    {
      query_period_ = query;
      delperiod = del;
      usefulperiod = useful;
      
      
    }
  
  

  
};

#endif
