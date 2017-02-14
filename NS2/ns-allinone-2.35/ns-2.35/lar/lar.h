/******************************************************************************
*   File Name: lar.h
*   Purpose: LAR protocol for ns-2 header file
*   Original Author: Jeff Boleng
*   Modified by: 
*   Date Created: Some time in 2002
*
*   Copyright (C) 2004  Toilers Research Group -- Colorado School of Mines
*
*   Please see COPYRIGHT.TXT and LICENSE.TXT for copyright and license
*   details.
*******************************************************************************/

#ifndef ns_lar_h
#define ns_lar_h

#include "object.h"
#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"
#include "mac.h"
#include "mobilenode.h"
#include "timer-handler.h"

#include <set>
#include <map>
#include <list>
#include <iostream>

using namespace std;

#define maxRouteLength 32

// This means that the jitter ranges from 0.0 to 0.0001 with
// a mean of 0.00005, so the average number of bits that could
// be transmitted without jitter is 50 per transmission.  This seems
// acceptable.
// Note:  this is just the divisor in the random number range calculation.
#define  larJitter 10000.0

#define lar_min(x,y) (((x) < (y)) ? (x) : (y))
#define lar_max(x,y) (((x) > (y)) ? (x) : (y))

// size of the fixed portion of the simulated header
#define LAR_RTREQ_HDR_SIZE_FIXED       38
#define LAR_RTREQ_FLOOD_HDR_SIZE_FIXED 26
#define LAR_RTREP_HDR_SIZE_FIXED       38
#define LAR_DATA_HDR_SIZE_FIXED        28
#define LAR_RTERR_HDR_SIZE_FIXED       38

// prototypes for utility functions
void LARprintHeaders(Packet *p);

struct hdr_lar {

  // 'R' = Route Request
  // 'A' = Route Reply (A for Answer)
  // 'D' = Data Packet
  // 'E' = Route Error (broken link)
  // 'U' = unset
  char larCode_;

  // 'F' = flood
  // 'B' = box
  // 'S' = step
  // 'N' = no forward used for the ringZeroSearch option
  // 'U' = unset
  char forwardCode_;

  // fields needed by route requests

  // fields needed by box and step forwarding methods
  double      sourceX_;
  double      sourceY_;
  double      lastHopX_;
  double      lastHopY_;
  double      destinationX_;
  double      destinationY_;
  nsaddr_t    destinationID_;
  double      sendTime_;
  bool        newPkt_;   // true if the packet came from the originator
                         // false if an intermediate node has taken it over

  // fields needed for Box forwarding method
  double      destinationSpeed_;      
  double      speedTime_;


  // fields needed by route replies / data packets / route error messages
  nsaddr_t  route_[maxRouteLength];  // currently nsaddr_t is a 32 bit int defined in
                                     // config.h
  int       hops_;
  nsaddr_t  requestID_;  // this is the packet ID of the route request that
                         // prompted this route reply packet

  double    requestSendTime_;  // used to calculate the total delay

  // fields needed by data packets
  int       dataLength_;   // length of the data in the packet in bytes
                           // does not include the header length

  // things I get for "free" from hdr_ip in packet.h
  // int saddr();      // IP addr of source sender
  // int sport();      // port number of the source sender
  // int daddr();      // IP addr of the destination
  // int dport();      // port of the destination

  // things I get for "free" from hdr_cmn in packet.h
  // double   ts_;            // timestamp
  // int      size_;          // simulated packet size
  // int      uid_;           // unique packet id
  // nsaddr_t prev_hop_;      // IP addr of forwarding hop
  // int      num_forwards_;  // number of forwards

  // Now I need to provide access functions for my structure members
  inline char     &larCode() { return larCode_; }
  inline char     &forwardCode() { return forwardCode_; }
  inline double   &sourceX() { return sourceX_; }
  inline double   &sourceY() { return sourceY_; }
  inline double   &lastHopX() { return lastHopX_; }
  inline double   &lastHopY() { return lastHopY_; }
  inline double   &destinationX() { return destinationX_; }
  inline double   &destinationY() { return destinationY_; }
  inline nsaddr_t &destinationID() { return destinationID_; }
  inline double   &destinationSpeed() { return destinationSpeed_; }
  inline double   &speedTime() { return speedTime_; }
  inline double   &sendTime() { return sendTime_; }
  inline bool     &newPkt() { return newPkt_; }
  inline int      &hops() { return hops_; }
  inline nsaddr_t &requestID() { return requestID_; }
  inline double   &requestSendTime() { return requestSendTime_; }
  inline int      &dataLength() { return dataLength_; }

  inline int size()
  {
    int s=0;
    if (larCode_ == 'R')
    {
      if (forwardCode_ == 'F')
      {
        s = LAR_RTREQ_FLOOD_HDR_SIZE_FIXED + 4 * hops_;
      } else {
        s = LAR_RTREQ_HDR_SIZE_FIXED + 4 * hops_;
      }
    } else if (larCode_ == 'D') {
      s = LAR_DATA_HDR_SIZE_FIXED + 4 * hops_ + dataLength_;
    } else if (larCode_ == 'A') {
      s = LAR_RTREP_HDR_SIZE_FIXED + 4 * hops_;
    } else if (larCode_ == 'E') {
      s = LAR_RTERR_HDR_SIZE_FIXED + 4 * hops_;
    }

    return s;
  }

  inline int headerLength()
  {
    int s=0;
    if (larCode_ == 'R')
    {
      if (forwardCode_ == 'F')
      {
        s = LAR_RTREQ_FLOOD_HDR_SIZE_FIXED + 4 * hops_;
      } else {
        s = LAR_RTREQ_HDR_SIZE_FIXED + 4 * hops_;
      }
    } else if (larCode_ == 'D') {
      s = LAR_DATA_HDR_SIZE_FIXED + 4 * hops_;
    } else if (larCode_ == 'A') {
      s = LAR_RTREP_HDR_SIZE_FIXED + 4 * hops_;
    } else if (larCode_ == 'E') {
      s = LAR_RTERR_HDR_SIZE_FIXED + 4 * hops_;
    }

    return s;
  }

  static int offset_;
  inline static int& offset()
  {
    return offset_;
  }
  inline static hdr_lar* access(const Packet *p)
  {
    return (hdr_lar*) p->access(offset_);
  }
  
};

class LARAgent;

typedef struct
{
  double x;
  double y;
  double v;
  char   method;  //flood, box, or step (F,B, or S)
  nsaddr_t  route[maxRouteLength];
  int hops;
  double timeOfLastRequest;
  double timeOfVelocity;
  bool requestPending;
  bool noRoute;
} larDestType;

// timer class needed for the route request timeout
class LARRouteRequestTimer : public TimerHandler
{
  public:
    LARRouteRequestTimer(LARAgent *a, nsaddr_t d, int t, larDestType *r, char m):  TimerHandler()
    {
      agent = a;
      dest = d;
      triesLeft = t;
      routeInfo = r;
      forwardMethod = m;
    }
    void setTriesLeft(int t) {triesLeft = t;}
    void setForwardMethod(char c) {forwardMethod = c;}
    void setRouteInfo(larDestType *r) { routeInfo = r; }
    virtual void expire(Event *e);
  protected:
//    virtual void expire(Event *e);
    LARAgent *agent;
    nsaddr_t dest;
    int triesLeft;
    larDestType *routeInfo;
    char forwardMethod;
};

// Agent inheritance is because this is a specialized Agent

// this will be needed when full up promiscuous mode is implemented
// class LARAgent : public Tap, public Agent
class LARAgent : public Agent
{
  public:
    LARAgent();
    ~LARAgent();
    int command(int argc, const char*const* argv);
    void recv(Packet*, Handler*);
    void routeError(Packet *p);

    // This method gets all the packet that the node's MAC layer hears
    // so it can promiscuously listen to them.
    //void tap(const Packet *p);

  protected:
    int off_lar_;
    bool inForwardingRegion(char fwdCode, double srcX, double srcY,
                            double destX, double destY, 
                            double destSpeed, double speedTime);
    int sendInitialRouteRequest(nsaddr_t id, larDestType *routeEntry);
    bool dupRcvdPacket(int id);
    bool dupSentPacket(int id);
    void recordSentPacket(int id);
    void recordRcvdPacket(int id);
    void sendPacket(Packet *p, nsaddr_t to);
    void broadcastPacket(Packet *p);
    void rebroadcastPacket(Packet *p);

    friend void LARRouteRequestTimer::expire(Event *e);

  private:
    void sendNewData(Packet *p, nsaddr_t destID);
    void sendRouteReply(Packet *p);
    void purgePendingPacketsFor(nsaddr_t id);
    void purgeAllPendingPackets();
    void addToPendingPackets(Packet *p);
    void checkPendingQueueForPackets(nsaddr_t id);

    void clearRequestPending(nsaddr_t id);
    void maintainRouteTable(Packet *p);
    bool subRoute(larDestType *rt, Packet *p);

    // this function looks up the current destination in the route table and calls
    // sendRouteRequest appropriately
    void routeRequest(Packet *p);
    int sendRouteRequest(nsaddr_t id, char fwdCode, larDestType *routeEntry, bool ringZero);

    // this is just a set of packets I've sent
    set<int, less<int> > sentPackets;
    // this is just a set of packets I've received
    set<int, less<int> > rcvdPackets;

    list<Packet*> pendingPackets;

    // this is the routing table.  It is indexed by destination node id
    map<nsaddr_t, larDestType, less<int> > routeTable;
    map<nsaddr_t, bool, less<int> > updatedRoute;

    // this is a list of route request timers indexed by the destination node id
    map<nsaddr_t, LARRouteRequestTimer*, less<int> > timers;

    NsObject *ll;  // link layer output in order to avoid using a routing
                   // agent
    MobileNode *node;  // the node that this agent is attached to

    // all these variables are for statistics keeping...
    int dataTxPkts;
    int dataRxPkts;
    int dataTxBytes;
    int dataRxBytes;
    double totalEEdelay;
    int totalHops;
    int protocolTxPkts;
    int protocolRxPkts;
    int protocolTxBytes;
    int protocolRxBytes;
    int dataTxPktsFwd;
    int dataRxPktsFwd;
    int dataTxBytesFwd;
    int dataRxBytesFwd;
    int dataDroppedSrc;
    int dataDroppedInt;
    int routeErrorsGen;
    int routeErrorsRx;
    int larRtReqTx;
    int larRtReqRx;
    int floodRtReqTx;
    int floodRtReqRx;
    int oneHopRtReqTx;
    int oneHopRtReqRx;
    int srcRtErrors;
    int droppedAtEnd;
    int pktOverflow;
};

#endif


