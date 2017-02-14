/******************************************************************************
*   File Name: lar.cc
*   Purpose: LAR protocol cc file for ns-2
*   Original Author: Jeff Boleng
*   Modified by: 
*   Date Created: Some time in 2002
*
*   Copyright (C) 2004  Toilers Research Group -- Colorado School of Mines
*
*   Please see COPYRIGHT.TXT and LICENSE.TXT for copyright and license
*   details.
*******************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <ctype.h>
#include <cmath>
#include "lar.h"

using namespace std;

int hdr_lar::offset_;

static class LARHeaderClass: public PacketHeaderClass
{
  public:
    LARHeaderClass():  PacketHeaderClass("PacketHeader/LAR",
                                          sizeof(hdr_lar))
    {
      bind_offset(&hdr_lar::offset_);
    }
} class_larhdr;

static class LARClass: public TclClass
{
  public:
    LARClass() : TclClass("Agent/LAR") {}
    TclObject* create(int, const char*const*)
    {
      return (new LARAgent());
    }
} class_lar;

// The next two class declarations bind our C++ classes to the OTcl
// class.
//static class LARHeaderClass : public PacketHeaderClass {
//public:
  //LARHeaderClass() : PacketHeaderClass("PacketHeader/LAR", 
					//sizeof(hdr_lar)) {}
//} class_larhdr;


//static class LARClass : public TclClass {
//public:
  //LARClass() : TclClass("Agent/LAR") {}
  //TclObject* create(int, const char*const*) {
    //return (new LARAgent());
  //}
//} class_lar;

// if a route reply is not recieved in this amount of time the
// request is flooded
double LARrouteRequestTimeout = 0.5;

// This is the lar "fudge factor".  It is used to make the box or
// step methods "slightly" bigger or smaller.  It is measured in units or meters
double LARDelta = 0.0;

// if true, this will add jitter to unicast packets being sent
bool LARuseJitteronSend = true;
bool LARuseJitteronBroadcast = true;

// timer to drop packets in the pending queue after
bool purgePending = true;
double dropPendingPacketsAfter = 30.0;

// if true, then the node will query immediate neighbors for a route before
// sending a normal LAR route request
bool useRingZeroSearch = true;
double ringZeroWait = 0.030;     // this is 10 milliseconds

bool usePromiscuousListening = false;

bool useIntermediateRouteRepair = true;

// This option and useRoutePersistence are mutually exclusive.  Only one can be
// true at a time, and this one is given preference.  Although both can be false.
// This option is also only used if the useIntermediateRouteRepair flag is set.
// Otherwise, intermediate packets are dropped on a route error no matter what.
bool dropIntermediatePacketsIfNoRoute = true;

// WARNING - this option isn't working correctly.  It works some, but creates too
//           much overhead.
bool useIntermediateRouteReply = false;

int pendingPacketQueueLength = 64;  // this is the maximum number of packets that
                                    // can be in the pending packet queue at one time
// set it to a big number that is more packets than you expect for an infinite queue

// these are used to set the timeout for continued flooding of route requests if there
// are packets still in the pending queue
//
// WARNING - this option doesn't work, and using it means the simulation will never
//           terminate!  I'll fix it soon.
bool useRoutePersistence = false;
// This option and dropIntermediatePacketsIfNoRoute are mutually exclusive.  Only one can be
// true at a time.  Although both can be false.
double LARroutePersistenceTimeout = 0.5;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

nsaddr_t routeBreakPoint = -77;

// used for reporting status, higher numbers imply more verbose output
int LARverbosity = 0;

// here is the implementation of the timer
void LARRouteRequestTimer::expire(Event *e)
{
  if (LARverbosity > 0)
  {
    cout << endl << "*******************************" << endl;
    cout << "In LARRouteRequestTimer::expire" << endl;
    cout << "Time = " << Scheduler::instance().clock() << endl;
  }

  if (triesLeft >= 1)
  {
    if (LARverbosity > 0)
    {
      cout << "Trying again with forward code " << forwardMethod << "..." << endl;
      cout << "destX:      " << routeInfo->x << endl;
      cout << "destY:      " << routeInfo->y << endl;
      cout << "destV:      " << routeInfo->v << endl;
      cout << "destVtime:  " << routeInfo->timeOfVelocity << endl;
    }

    triesLeft--;
    // no need to reschedule a new timer because sendRouteRequest does that
    //agent->timers[this->dest]->resched(LARrouteRequestTimeout);
    agent->sendRouteRequest(this->dest, forwardMethod, this->routeInfo, false);
  } else {
    if (LARverbosity > 0)
    {
      cout << "Didn't find a route, emptying the pending packet queue." << endl;
      cout << "My address:    " << this->agent->addr() << endl;
      cout << "Dest address:  " << this->dest << endl;
    }
    // if this is not the first try on the timer, then the pending route queue needs to be
    // emptied of it's contents because we've failed to find a route twice...
    triesLeft = 0;
    agent->clearRequestPending(this->dest);
    if (purgePending)
    {
      agent->purgePendingPacketsFor(this->dest);
    }
    if (useRoutePersistence)
    {
      agent->checkPendingQueueForPackets(this->dest);
    }
  }
  if (LARverbosity > 0)
  {
    cout << "Done in LARRouteRequestTimer::expire" << endl;
    cout << "*******************************" << endl << endl;
  }
  return;
}


// ***********************************************************************
//                            Creator
// ***********************************************************************
LARAgent::LARAgent() : Agent(PT_LAR)
{
  //off_lar_ = hdr_lar::offset();
  //bind("packetSize_", &size_);
  //bind("off_lar_", &off_lar_);
  // This binds the C++ and OTcl variables.  I may need to do some
  // more, but I don't know right now.
  
  ll = NULL;
  node = NULL;

  dataTxPkts = 0;
  dataRxPkts = 0;
  dataTxBytes = 0;
  dataRxBytes = 0;
  totalEEdelay = 0.0;
  totalHops = 0;
  protocolTxPkts = 0;
  protocolRxPkts = 0;
  protocolTxBytes = 0;
  protocolRxBytes = 0;
  dataTxPktsFwd = 0;
  dataRxPktsFwd = 0;
  dataTxBytesFwd = 0;
  dataRxBytesFwd = 0;
  dataDroppedSrc = 0;
  dataDroppedInt = 0;
  routeErrorsGen= 0;
  routeErrorsRx = 0;
  larRtReqTx = 0;
  larRtReqRx = 0;
  floodRtReqTx = 0;
  floodRtReqRx = 0;
  oneHopRtReqTx = 0;
  oneHopRtReqRx = 0;
  srcRtErrors = 0;
  droppedAtEnd = 0;
  pktOverflow = 0;

  srand(clock());

  return;
}

// ***********************************************************************
//                            Destructor
// ***********************************************************************
LARAgent::~LARAgent()
{
  rcvdPackets.erase(rcvdPackets.begin(), rcvdPackets.end());
  sentPackets.erase(sentPackets.begin(), sentPackets.end());
  routeTable.erase(routeTable.begin(), routeTable.end());
  updatedRoute.erase(updatedRoute.begin(), updatedRoute.end());
  timers.erase(timers.begin(), timers.end());
  pendingPackets.erase(pendingPackets.begin(), pendingPackets.end());
  return;
}

void larRouteErrorCallback(Packet *p, void *arg)
{
  ((LARAgent*) arg)->routeError(p);
}

// ***********************************************************************
//                            command
// ***********************************************************************
int LARAgent::command(int argc, const char*const* argv)
{
  TclObject *obj;  

  if (argc == 2)
  {
    if (strcmp(argv[1], "larDone") == 0)
    {
      // this purges any pending packets at the end of the run, just in case
      this->purgeAllPendingPackets();

      cout << endl << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
      cout << "LAR agent on node " << this->addr() << " done at "
           << Scheduler::instance().clock() << endl;
      cout << "Statistics for node:  " << this->addr() << endl;
      cout << "Data Received:" << endl;
      cout << "\tdataRxPkts:       " << dataRxPkts << endl;
      cout << "\tdataRxBytes:      " << dataRxBytes << endl;
      cout << "\ttotalEEdelay:     " << totalEEdelay << endl;
      cout << "\ttotalHops:        " << totalHops << endl;
      if (dataRxPkts > 0)
      {
        cout << "\t~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        cout << "\tAverage EE delay:   " << (double)totalEEdelay/(double)dataRxPkts << endl;
        cout << "\tAverage Hops:       " << (double)totalHops/(double)dataRxPkts << endl;
        if(dataTxPkts >0){
            cout<<"\tAverage Delivery ratio:    "<<(double)dataRxPkts/(double)dataTxPkts << endl;
      }        

      }
      cout << "Data Sent or Dropped:" << endl;
      cout << "\tdataTxPkts:       " << dataTxPkts << endl;
      cout << "\tdataTxBytes:      " << dataTxBytes << endl;
      cout << "\tdataDroppedSrc:   " << dataDroppedSrc << endl;
      cout << "\tdataDroppedInt:   " << dataDroppedInt << endl;
      cout << "Data Overhead:" << endl;
      cout << "\tdataTxPktsFwd:    " << dataTxPktsFwd << endl;
      cout << "\tdataTxBytesFwd:   " << dataTxBytesFwd << endl;
      cout << "\tdataRxPktsFwd:    " << dataRxPktsFwd << endl;
      cout << "\tdataRxBytesFwd:   " << dataRxBytesFwd << endl;
      cout << "Protocol Overhead:" << endl;
      cout << "\tprotocolTxPkts:   " << protocolTxPkts << endl;
      cout << "\tprotocolTxBytes:  " << protocolTxBytes << endl;
      cout << "\tprotocolRxPkts:   " << protocolRxPkts << endl;
      cout << "\tprotocolRxBytes:  " << protocolRxBytes << endl;
      cout << "\trouteErrorsGen:   " << routeErrorsGen<< endl;
      cout << "\trouteErrorsRx:    " << routeErrorsRx << endl;
      cout << "\tlarRtReqTx:       " << larRtReqTx << endl;
      cout << "\tlarRtReqRx:       " << larRtReqRx << endl;
      cout << "\tfloodRtReqTx:     " << floodRtReqTx << endl;
      cout << "\tfloodRtReqRx:     " << floodRtReqRx << endl;
      cout << "\toneHopRtReqTx:    " << oneHopRtReqTx << endl;
      cout << "\toneHopRtReqRx:    " << oneHopRtReqRx << endl;
      cout << "\tsrcRtErrors:      " << srcRtErrors << endl;
      cout << "\tdroppedAtEnd:     " << droppedAtEnd << endl;
      cout << "\tpktOverflow:      " << pktOverflow << endl;
      cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl << endl;
      return (TCL_OK);
    }
  } else if (argc == 3) {
    if (strcmp(argv[1], "setVerbosity") == 0)
    {
      LARverbosity = atoi(argv[2]);
      if (LARverbosity < 0)
      {
        LARverbosity = 0;
      } else if (LARverbosity > 2) {
        LARverbosity = 2;
      }
      return (TCL_OK);
    } else if (strcmp(argv[1], "set-ll") == 0) {
      if( (obj = TclObject::lookup(argv[2])) == 0)
      {
        fprintf(stderr, "LARAgent(set-ll): %s lookup of %s failed\n", 
                        argv[1], argv[2]);
        return (TCL_ERROR);
      }
      ll = (NsObject*) obj;
      return (TCL_OK);
    } else if (strcmp(argv[1], "set-node") == 0) {
      if( (obj = TclObject::lookup(argv[2])) == 0)
      {
        fprintf(stderr, "LARAgent(set-node): %s lookup of %s failed\n", 
                        argv[1], argv[2]);
        return (TCL_ERROR);
      }
      node = dynamic_cast< MobileNode * >(obj);
      if (node)       // dynamic cast was successful and didn't return NULL
      {
        return (TCL_OK);
      } else {
        fprintf(stderr, "Unable to dynamically cast %s to a MobileNode\n",
                        argv[2]);
        return (TCL_ERROR);
      } 
    } else if (strcmp(argv[1], "setRouteRequestTimeout") == 0) {
      LARrouteRequestTimeout = atoi(argv[2]);
      return (TCL_OK);
    } else if (strcmp(argv[1], "setLARDelta") == 0) {
      LARDelta = atof(argv[2]);
      if (LARverbosity > 0)
      {
        fprintf(stderr, "Set LARDelta to %f at node %d.\n", LARDelta, this->addr());
      }
      return (TCL_OK);
    }
  } else if (argc == 5) {
    if (strcmp(argv[1], "sendData") == 0)
    {
      if ((ll == NULL) || (node == NULL))
      {
        fprintf(stderr, "Link layer and node must be set on a lar agent\n");
        fprintf(stderr, "before anything can be sent.\n");
        return (TCL_ERROR);
      }
      nsaddr_t destID = (nsaddr_t)atoi(argv[2]);
      node->update_position();

      // Create a new packet
      Packet* dataPkt = allocpkt();
      // Access the LAR header for the new packet:
      // new way to do this (2.1b7 and later)
      struct hdr_lar* larhdr = hdr_lar::access(dataPkt);
      struct hdr_cmn* hdrcmn = hdr_cmn::access(dataPkt);

      // old way to do this
      //hdr_lar* larhdr = (hdr_lar*)dataPkt->access(off_lar_);
      //hdr_cmn* hdrcmn = (hdr_cmn*)dataPkt->access(off_cmn_);

      hdrcmn->xmit_failure_ = larRouteErrorCallback;
      hdrcmn->xmit_failure_data_ = (void *) this;

      // Store the current time in the 'send_time' field
      larhdr->sendTime_ = Scheduler::instance().clock();

      larhdr->larCode_ = 'D';
      larhdr->forwardCode_ = toupper(argv[4][0]);
      //cout << "forwardCode = " << larhdr->forwardCode() << endl;
      larhdr->sourceX_ = node->X();
      larhdr->sourceY_ = node->Y();
      larhdr->lastHopX_ = node->X();
      larhdr->lastHopY_ = node->Y();
      larhdr->destinationID_ = destID;
      larhdr->dataLength_ = atoi(argv[3]);
      larhdr->newPkt_ = true;

      larhdr->requestID_ = 0;
      larhdr->requestSendTime_ = 0;

      sendNewData(dataPkt, destID);

      // return TCL_OK, so the calling function knows that the
      // command has been processed
      return (TCL_OK);
    } // if (strcmp(argv[1], "sendData") == 0)
  } // } else if (argc == 5) {
  // If the command hasn't been processed by LARAgent()::command,
  // call the command() function for the base class
  return (Agent::command(argc, argv));
}


// ***********************************************************************
//                            recv
// ***********************************************************************
void LARAgent::recv(Packet* pkt, Handler*)
{
  // Access the common header for the received packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(pkt);
  struct hdr_cmn* hdrcmn = hdr_cmn::access(pkt);
  struct hdr_ip* hdrip = hdr_ip::access(pkt);

  // old way to do this
  //hdr_cmn* hdrcmn = (hdr_cmn*)pkt->access(off_cmn_);

  // Access the IP header for the received packet:
  //hdr_ip* hdrip = (hdr_ip*)pkt->access(off_ip_);

  // Access the LAR header for the received packet:
  //hdr_lar* larhdr = (hdr_lar*)pkt->access(off_lar_);

  node->update_position();
  // Set the newPkt field to false.  Since this packet has been received,
  // it can't be new.
  larhdr->newPkt() = false;

  if (larhdr->larCode() == 'D')
  {
    // only do this if this packet is not destined for me
    if (this->addr() != larhdr->destinationID())
    {
      dataRxPktsFwd++;
      dataRxBytesFwd += larhdr->dataLength();
    }
    // count the protocol header bytes as control bytes for
    // all packets, even data
    protocolRxBytes += larhdr->headerLength();
  } else {
    protocolRxPkts++;
    protocolRxBytes += larhdr->headerLength();
  }
  // Is it a duplicate packet?
  if (this->dupRcvdPacket(hdrcmn->uid_) || this->dupSentPacket(hdrcmn->uid_))
  {
    // already seen it, or I sent it, just free it and drop it
    if (LARverbosity == 2)
    {
      cout << "****************************************************" << endl;
      cout << "In LAR recv at node:  " << this->addr()
           << " at [" << node->X() << "," << node->Y() << "]" << endl;
      cout << "Time:  " << Scheduler::instance().clock() << endl;
      cout << "Route in header:  ";
      for (int i=0; i<larhdr->hops(); i++)
      {
        cout << larhdr->route_[i] << "  ";
      }
      cout << endl;
      cout << "Packet ID:  " << hdrcmn->uid_ 
           << "     Type:  " << larhdr->larCode() << endl;
      cout << "Duplicate packet seen from " << hdrcmn->prev_hop_ 
           << ", freeing it." << endl;
      cout << "Returning from recv early" << endl;
      cout << "Time:  " << Scheduler::instance().clock() << endl;
      cout << "****************************************************" << endl << endl;
    }
    Packet::free(pkt);
    return;
  } else {
    // This is redundant for now because dupRcvdPacket records the packet
    // as recieved, but I don't like that side effect and may change it
    // later.
    recordRcvdPacket(hdrcmn->uid_);
  }

  if (LARverbosity > 0)
  {
    cout << "****************************************************" << endl;
    cout << "In LAR recv at node:  " << this->addr()
         << " at [" << node->X() << "," << node->Y() << "]" << endl;
    cout << "Time:  " << Scheduler::instance().clock() << endl;
    cout << "Route in header:  ";
  
    for (int i=0; i<larhdr->hops(); i++)
    {
      cout << larhdr->route_[i] << "  ";
    }
    cout << endl;
    cout << "Packet Type:  " << larhdr->larCode() << endl;
    cout << "Forwarding Code:  " << larhdr->forwardCode() << endl;
  } // if (LARverbosity > 0)

  nsaddr_t dest = larhdr->destinationID();
  map<int, larDestType, less<int> >::iterator  destPtr;
  destPtr = routeTable.find(dest);
  // What kind of packet is it?
  // ***********************************************************************
  //                            Route Request
  // ***********************************************************************
  if (larhdr->larCode() == 'R')
  {
    if(LARverbosity > 0)
    {
      cout << "Route Request packet seen" << endl;
      cout << "From:       " << hdrip->src_.addr_ 
           << " at [" << larhdr->sourceX() << "," << larhdr->sourceY() << "]" << endl;
      cout << "Last hop:   " << hdrcmn->prev_hop_ << endl;
      cout << "Packet ID:  " << hdrcmn->uid_ << endl;
    }
//LARprintHeaders(pkt);
    if (this->addr() == larhdr->destinationID())
    {
      if (larhdr->forwardCode() == 'F')
      {
        floodRtReqRx++;
      } else {
        larRtReqRx++;
      }
      // this is a route request for a route to me
      // I need to reply with a route reply
      // Create a new packet
      Packet* newpkt = allocpkt();
      // Access the headers for the new packet:
      // new way to do this (2.1b7 and later)
      struct hdr_lar* newlarhdr = hdr_lar::access(newpkt);
      struct hdr_cmn* newhdrcmn = hdr_cmn::access(newpkt);

      // old way to do this
      //hdr_lar* newlarhdr = (hdr_lar*)newpkt->access(off_lar_);
      newlarhdr->hops() = 0;
      //hdr_cmn* newhdrcmn = (hdr_cmn*)newpkt->access(off_cmn_);

      newlarhdr->larCode_ = 'A';
      newlarhdr->forwardCode_ = larhdr->forwardCode();
      newlarhdr->sourceX_ = larhdr->sourceX_;
      newlarhdr->sourceY_ = larhdr->sourceY_;
      newlarhdr->lastHopX_ = node->X();
      newlarhdr->lastHopY_ = node->Y();
      newlarhdr->destinationX_ = node->X();
      newlarhdr->destinationY_ = node->Y();
      newlarhdr->destinationID_ = larhdr->route_[0];
      newlarhdr->sendTime_ = Scheduler::instance().clock();
      newlarhdr->newPkt_ = true;
      newlarhdr->destinationSpeed_ = node->speed();
      newlarhdr->speedTime_ = Scheduler::instance().clock();
      // puts the route that the join demand took in the join table
      for (int i=0; i<larhdr->hops(); i++)
      {
        newlarhdr->route_[i] = larhdr->route_[i];
      }
      newlarhdr->hops() = larhdr->hops();

      // adds the final destination to the route
      newlarhdr->route_[newlarhdr->hops()] = this->addr();
      newlarhdr->hops()++;
      newlarhdr->requestID_ = hdrcmn->uid_;
      newlarhdr->requestSendTime_ = larhdr->requestSendTime();
      newlarhdr->dataLength_ = 0;

      nsaddr_t srcID = larhdr->route_[larhdr->hops()-1];
      if (LARverbosity > 0)
      {
        cout << "Sending a Route Reply, ID:  " << newhdrcmn->uid_ 
             << " to " << srcID << endl;
      }
      //this->broadcastPacket(newpkt);
      // Don't want to rebroadcast, want send it to my "upstream" neighbor

      // this is a Route Request intended for me, so I should put it in my
      // route table no matter what
      //if (usePromiscuousListening)
      //{
        maintainRouteTable(newpkt);
      //}
      this->sendPacket(newpkt, srcID);
      Packet::free(pkt);
      return;
    } else if (useIntermediateRouteReply && (destPtr != routeTable.end())) {
//cerr << "One:     " << flush;
      this->sendRouteReply(pkt);
      Packet::free(pkt);
      return;
    } else if ((larhdr->forwardCode() == 'N') && (destPtr != routeTable.end())) {
      // I must be a direct neighbor of the node doing a route request
      // this depends on short circuit boolean...  If the first condition isn't true, the
      // second condition will create an access fault
//cerr << "Two:     " << flush;
      this->sendRouteReply(pkt);
      Packet::free(pkt);
      return;
    } else if (this->inForwardingRegion(larhdr->forwardCode(), 
                                        larhdr->lastHopX(), larhdr->lastHopY(),
                                        larhdr->destinationX(), larhdr->destinationY(),
                                        larhdr->destinationSpeed(), larhdr->speedTime())) {
      // re-broadcast the packet, because I'm in the forwarding region.
      if (LARverbosity > 0)
      {
        cout << "Rebroadcasting, forwarding node, but not the destination..." << endl;
        cout << "last hop location:     (" << larhdr->lastHopX() << "," << larhdr->lastHopY()
             << ")" << endl;
        cout << "destination location:  (" << larhdr->destinationX() << "," << larhdr->destinationY()
             << ")" << endl;
        cout << "destination speed:      " << larhdr->destinationSpeed() << endl;
        cout << "speed time:             " << larhdr->speedTime() << endl;
      }
      larhdr->lastHopX_ = node->X();
      larhdr->lastHopY_ = node->Y();
      larhdr->route_[larhdr->hops()] = this->addr();
      larhdr->hops()++;
      //if (usePromiscuousListening)
      //{
        // this is a Route Request, and I'm a flooding node in the forwarding
        // region, so I should update my route table regardless of if this is true.
        maintainRouteTable(pkt);
      //}
      this->rebroadcastPacket(pkt);
      return;
    } else {
      if (LARverbosity > 0)
      {
	cout << "Not in forwarding region and not the destination.  Ignoring." << endl;
      }
      // don't really need an else here because I take care of duplicate
      // packets up above
      Packet::free(pkt);
      return;
    }
  // ***********************************************************************
  //                            Route Reply
  // ***********************************************************************
  } else if (larhdr->larCode() == 'A') {
    if (LARverbosity > 0 )
    {
      cout << "Route Reply packet seen" << endl;
      cout << "From:       " << hdrip->src_.addr_ 
           << " at [" << larhdr->sourceX() << "," << larhdr->sourceY() << "]" << endl;
      cout << "Last hop:   " << hdrcmn->prev_hop_ << endl;
      cout << "Packet ID:  " << hdrcmn->uid_ << endl;
      cout << "Request ID:  " << larhdr->requestID_ << endl;
    }
    
    if (this->addr() == larhdr->destinationID())
    {
      // I sent the corresponding route request, and the reply just got back.
      if (LARverbosity > 0)
      {
        cout << "Back at originating node!" << endl;
        cout << "Route Request sent at " << larhdr->requestSendTime_
             << " received at " << Scheduler::instance().clock() << endl;
        cout << "Round trip delay = "  
             << Scheduler::instance().clock() - larhdr->requestSendTime_ << endl;
      }
      // up date the routing table entry and check the pending packets list for
      // any data packets waiting for this route

      maintainRouteTable(pkt);
      nsaddr_t destID = larhdr->route_[larhdr->hops()-1];
      if (updatedRoute[destID])
      {
        // now, check and see if there are packets waiting for this route
        // if we got a route reply, there must have been a route request, and there
        // probably are packets pending
        destPtr = routeTable.find(destID);
        list<Packet*>::iterator pendingPtr;
        for (pendingPtr=pendingPackets.begin(); pendingPtr!=pendingPackets.end(); /*nothing*/)
        {
          // Access the headers for the new packet:
          // new way to do this (2.1b7 and later)
          struct hdr_lar* pendinglarhdr = hdr_lar::access((*pendingPtr));

          // old way to do this
          //hdr_lar* pendinglarhdr = (hdr_lar*)(*pendingPtr)->access(off_lar_);

          if (LARverbosity > 0)
          {
            cout << "Checking pending packet addressed to " << pendinglarhdr->destinationID() << endl;
          }
          if (pendinglarhdr->destinationID() == destID)
          {
            if (LARverbosity > 0)
            {
              cout << "Sending a pending packet to " << destID << endl;
            }
            // if the pending packet is destined to the node we just received a route request
            // from, then flesh out the header and send it off
            pendinglarhdr->destinationX_ = destPtr->second.x;
            pendinglarhdr->destinationY_ = destPtr->second.y;
            pendinglarhdr->destinationSpeed_ = destPtr->second.v;
            pendinglarhdr->speedTime_ = destPtr->second.timeOfVelocity;
            pendinglarhdr->requestSendTime_ = destPtr->second.timeOfLastRequest;
            pendinglarhdr->hops_ = destPtr->second.hops;
            if (LARverbosity > 0)
            {
              cout << "Using route:  ";
            }
            for (int k=0; k<destPtr->second.hops; k++)
            {
              pendinglarhdr->route_[k] = destPtr->second.route[k];
              if (LARverbosity > 0)
              {
                cout << destPtr->second.route[k] << " ";
              }
            } // for (int k=0; k<destPtr->second.hops; k++)
            if (LARverbosity > 0)
            {
              cout << endl;
            }
            // send it out
            if (LARverbosity > 0)
            {
              cout << "****************************************************" << endl;
              cout << "Just received a route reply, so sending queued lar data from:  " << this->addr()
                   << " at [" << node->X() << "," << node->Y() << "]" << endl;
              cout << "Time:        " << Scheduler::instance().clock() << endl;
              cout << "Data size:   " << pendinglarhdr->dataLength() << endl;
              cout << "Total size:  " << pendinglarhdr->size() << endl;
            } // if (LARverbosity > 0)
            this->sendPacket((*pendingPtr), destPtr->second.route[1]);
            if (LARverbosity > 0)
            {
              cout << "****************************************************" << endl << endl;
            } // if (LARverbosity > 0)
            pendingPackets.erase(pendingPtr++);
          } else {
            pendingPtr++;
          } // if (pendinglarhdr->destinationID() == destID)
        } // for (pendingPtr=pendingPackets.begin(); pendingPtr!=pendingPackets.end(); /*nothing*/)

        // clear the flood timer for this destination if it exists
        map<nsaddr_t, LARRouteRequestTimer*, less<int> >::iterator timerPtr;
        timerPtr = timers.find(destID);
        if (timerPtr != timers.end())
        {
          if (LARverbosity > 0)
          {
            cout << "Cancelling the flood timer for this data packet." << endl;
          }
          timerPtr->second->force_cancel();
        } // if (timerPtr != timers.end())
      } else {
        // the packet didn't update the route table, so I still need a new route
        // basically, I should just return and let the timer expire
        if (LARverbosity > 0)
        {
          cout << "The route reply didn't update the routing table!" << endl;
        }
      }
      Packet::free(pkt);
      return;
    } else {
      // if I get a route reply and am not the sender, then I must be
      // the addressee of the route reply, and I should forward it back downstream
      nsaddr_t dest = -5;
      for (int i=0; i<larhdr->hops(); i++)
      {
        if (larhdr->route_[i] == this->addr())
        {
          // after the find, dest is pointing to my address, so decrement it to 
          // the previous one in the route
          dest = larhdr->route_[i-1];
          break;
        }
      }
      if (LARverbosity > 0)
      {
        cout << "Re-sending a Route Request, ID:  " << hdrcmn->uid_ 
             << " to " << dest << endl;
      }
      if (dest != -5)
      {
        larhdr->lastHopX_ = node->X();
        larhdr->lastHopY_ = node->Y();
        //if (usePromiscuousListening)
        //{
          // this is a Route Reply and I'm on the reverse source route, so I
          // should update my route table no matter what
          maintainRouteTable(pkt);
        //}
        this->sendPacket(pkt, dest);
        return;
      } else {
        cerr << "I'm supposed to forward this route reply back upstream, but I'm not in the" << endl;
        cerr << "recorded route!  I'm freeing the packet and returning.  In Route Reply code." << endl;
        cerr << "Pkt id:  " << hdrcmn->uid_ << "     My id:  " << this->addr() << "     Route:  ";
        for (int i=0; i<larhdr->hops(); i++)
        {
          cerr << larhdr->route_[i] << " ";
        }
        cerr << endl;
        Packet::free(pkt);
        return;
      }
    } // else of if (this->addr() == larhdr->destinationID())
  // ***********************************************************************
  //                            Data Packet
  // ***********************************************************************
  } else if (larhdr->larCode() == 'D') {
    if (LARverbosity > 0)
    {
      cout << "LAR data packet seen" << endl;
      cout << "From:       " << hdrip->src_.addr_ 
           << " at [" << larhdr->sourceX() << "," << larhdr->sourceY() << "]" << endl;
      cout << "Last hop:   " << hdrcmn->prev_hop_ << endl;
      cout << "Packet ID:  " << hdrcmn->uid_ << endl;
    }
    if (usePromiscuousListening)
    {
      // if I'm on the source route, or the source or destination, I should
      // already have all this routing information, so this should not be needed
      // anywhere else
      maintainRouteTable(pkt);
    }

    if (this->addr() == larhdr->destinationID())
    {
      // print some nice informational message
      if (LARverbosity > 0)
      {
        cout << "Recieved a data packet sucessfully!!!" << endl;
        cout << "Send scheduled at:    " << larhdr->sendTime() << endl;
        cout << "Received at:          " << Scheduler::instance().clock() << endl;
        cout << "Total Delay:          " 
             << Scheduler::instance().clock()-larhdr->sendTime() << endl;
	cout << "****************************************************" << endl << endl;
      }
      dataRxPkts++;
      dataRxBytes += larhdr->dataLength();

      // again, count all header bytes as control overhead
      protocolRxBytes += larhdr->headerLength();

      totalEEdelay += Scheduler::instance().clock()-larhdr->sendTime();
      totalHops += larhdr->hops() - 1;

      // Discard the packet
      Packet::free(pkt);
      return;
    } else {
      // if I get it, and it's not for me, then I must be on the route and I need
      // to forward it to the next down stream neighbor
      nsaddr_t dest = -5;
      for (int i=0; i<larhdr->hops(); i++)
      {
        if (larhdr->route_[i] == this->addr())
        {
          // after the find, dest is pointing to my address, so increment it to 
          // the previous one in the route
          dest = larhdr->route_[i+1];
          break;
        }
      }
      if (LARverbosity > 0)
      {
        cout << "Forwarding the data packet " << hdrcmn->uid_ 
             << " downstream to " << dest << endl;
      }
      if (dest != -5)
      {
        larhdr->lastHopX_ = node->X();
        larhdr->lastHopY_ = node->Y();
        // the transmit failure callback node pointer must be changed to me before resending
        // this is what generates a route error if when I send this packet there is a link
        // layer error returned
        hdrcmn->xmit_failure_data_ = (void *) this;
        this->sendPacket(pkt, dest);
        return;
      } else {
        cerr << "I'm supposed to forward this data packet downstream, but I'm not in the" << endl;
        cerr << "recorded route!  I'm freeing the packet and returning. In Data Packet code." << endl;
        Packet::free(pkt);
	cout << "****************************************************" << endl << endl;
        return;
      }
    }
  // ***********************************************************************
  //                            Route Error Packet
  // ***********************************************************************
  } else if (larhdr->larCode() == 'E') {
    if (LARverbosity > 0)
    {
      cout << "LAR route error packet seen" << endl;
      cout << "From:       " << hdrip->src_.addr_ 
           << " at [" << larhdr->sourceX() << "," << larhdr->sourceY() << "]" << endl;
      cout << "Last hop:   " << hdrcmn->prev_hop_ << endl;
      cout << "Packet ID:  " << hdrcmn->uid_ << endl;
    }
    if (usePromiscuousListening)
    {
      // route error
      maintainRouteTable(pkt);
    }
    if (this->addr() == larhdr->destinationID())
    {
      routeErrorsRx++;
      // this route error message is for me and I should initiate another route request 
      routeRequest(pkt);
      return;
    } else { // if (this->addr() == larhdr->destinationID())
      // this isn't for me directly, but I must be on the reverse route, so I 
      // should just forward it to the next destination on the route.

      // I should do this no matter what since I'm on the reverse route
      maintainRouteTable(pkt);
      nsaddr_t dest = -5;
      for (int i=0; i<larhdr->hops(); i++)
      {
        if (larhdr->route_[i] == this->addr())
        {
          // after the find, dest is pointing to my address, so decrement it to 
          // the previous one in the route
          dest = larhdr->route_[i-1];
          break;
        } // if (larhdr->route_[i] == this->addr())
      } // for (int i=0; i<larhdr->hops(); i++)
      if (LARverbosity > 0)
      {
        cout << "Forwarding the data packet " << hdrcmn->uid_ 
             << " upstream to " << dest << endl;
      } // if (LARverbosity > 0)
      if (dest != -5)
      {
        larhdr->lastHopX_ = node->X();
        larhdr->lastHopY_ = node->Y();
        this->sendPacket(pkt, dest);
        return;
      } else {
        cerr << "I'm supposed to forward this route error downstream, but I'm not in the" << endl;
        cerr << "recorded route!  I'm freeing the packet and returning.  In route error." << endl;
        Packet::free(pkt);
	cout << "****************************************************" << endl << endl;
        return;
      } // if (dest != -5)
    } // else of if (this->addr() == larhdr->destinationID())
  } else if (larhdr->larCode() == 'U') {
    // there is an error if this happens, but I'm not sure how to
    // respond, so it is commented out for now
    cerr << "The lar code is unset, so I'm freeing the packet." << endl;
    Packet::free(pkt);
    return;
  }
  if (LARverbosity > 0)
  {
    cout << "Returning from recv at the bottom" << endl;
    cout << "Time:  " << Scheduler::instance().clock() << endl;
    cout << "****************************************************" << endl << endl;
  }

  return;
}

// ***********************************************************************
//                            maintainRouteTable
// ***********************************************************************
void LARAgent::maintainRouteTable(Packet *p)
{
  // clear all the flags in updatedRoute
  map<nsaddr_t, bool, less<int> >::iterator flagPtr;
  for (flagPtr=updatedRoute.begin(); flagPtr!=updatedRoute.end(); flagPtr++)
  {
    flagPtr->second = false;
  }
  // Access the LAR header for the received packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(p);

  // old way to do this
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  if (LARverbosity > 0)
  {
    cout << "in maintainRouteTable of = " << this->addr() << endl;
  }
  int myIndex;
  bool myIndexIsInRoute = false;
  for (myIndex=0; myIndex<larhdr->hops(); myIndex++)
  {
    if (larhdr->route_[myIndex] == this->addr())
    {
      if (LARverbosity > 0)
      {
        cout << "Found my address in the route at index " << myIndex << endl;
      }
      myIndexIsInRoute = true;
      break;
    }
  }

  if (myIndexIsInRoute)
  {
    map<nsaddr_t, larDestType, less<int> >::iterator  destPtr;
    nsaddr_t dest;
    // **********************************************************
    //                 route error, invalidate routes
    // **********************************************************
    if (larhdr->larCode() == 'E')
    {
      // this is a route error, so I have to invalidate all the routes from
      // me to all the destinations after the break point
      int breakIndex;
      for (breakIndex=0; breakIndex<larhdr->hops(); breakIndex++)
      {
        if (larhdr->route_[breakIndex] == routeBreakPoint)
        {
          break;
        }
      }
      for (int e=(breakIndex+1); e<larhdr->hops(); e++)
      {
        dest = larhdr->route_[e];
        destPtr = routeTable.find(dest);
        if (destPtr != routeTable.end())
        {
          destPtr->second.noRoute = true;
          updatedRoute[dest] = true;
        }
      }
    } else {
      int j, k;
      int entries=1;
    // **********************************************************
    //                 update routes going up the route
    // **********************************************************
      for (j=(myIndex+1); j<larhdr->hops(); j++)
      {
        entries++;
        dest = larhdr->route_[j];
        destPtr = routeTable.find(dest);
        if (destPtr == routeTable.end())
        {
          if (LARverbosity > 0)
          {
            cout << "(up) This route entry didn't exist, creating a new entry for " << dest << endl;
            cout << "Route:  ";
            for (k=0; k<entries; k++)
            {
              cout << larhdr->route_[myIndex+k] << "  ";
            }
            cout << endl;
          }
          // this is a new entry in the route table
          larDestType newEntry;
          newEntry.x = larhdr->destinationX();
          newEntry.y = larhdr->destinationY();
          newEntry.v = larhdr->destinationSpeed();
          newEntry.method = larhdr->forwardCode();
  
          newEntry.hops = entries;
          for (k=0; k<entries; k++)
          {
            newEntry.route[k] = larhdr->route_[myIndex+k];
          }
          newEntry.timeOfLastRequest = larhdr->requestSendTime();
          newEntry.timeOfVelocity = larhdr->speedTime();
          newEntry.requestPending = false;
          newEntry.noRoute = false;
          routeTable[dest] = newEntry;
          updatedRoute[dest] = true;
        } else {
          if (LARverbosity > 0)
          {
            cout << "(up) This route entry did exist, checking the time stamp." << endl;
            cout << "Route:  ";
            for (k=0; k<entries; k++)
            {
              cout << larhdr->route_[myIndex+k] << "  ";
          }
          cout << endl;
          }
          if (larhdr->speedTime() > destPtr->second.timeOfVelocity)
          {
            if (LARverbosity > 0)
            {
              cout << "(up) The time stamp is newer, so I'm using this one." << endl;
            }
            // this entry existed in the route table and we just need to update the information
            destPtr->second.x = larhdr->destinationX();
            destPtr->second.y = larhdr->destinationY();
            destPtr->second.v = larhdr->destinationSpeed();
            destPtr->second.method = larhdr->forwardCode();
    
            destPtr->second.hops = entries;
            for (k=0; k<entries; k++)
            {
              destPtr->second.route[k] = larhdr->route_[myIndex+k];
            }
            destPtr->second.timeOfLastRequest = larhdr->requestSendTime();
            destPtr->second.timeOfVelocity = larhdr->speedTime();
            destPtr->second.requestPending = false;
            destPtr->second.noRoute = false;
            updatedRoute[dest] = true;
          } else {
            if (LARverbosity > 0)
            {
              //cout << "(up) The time stamp is NOT newer, so re-setting the requestPending flag." << endl;
              cout << "(up) The time stamp is NOT newer." << endl;
            }
            //destPtr->second.requestPending = false;
            //destPtr->second.noRoute = true;
            //updated = false;
          }
        }
      }

      entries=1;
    // **********************************************************
    //                 update routes going down the route
    // **********************************************************
      for (j=(myIndex-1); j>=0; j--)
      {
        entries++;
        dest = larhdr->route_[j];
        destPtr = routeTable.find(dest);
        if (destPtr == routeTable.end())
        {
          if (LARverbosity > 0)
          {
            cout << "(down) This route entry didn't exist, creating a new entry for " << dest << endl;
            cout << "Route:  ";
            for (k=0; k<entries; k++)
            {
              cout << larhdr->route_[myIndex-k] << "  ";
            }
            cout << endl;
          }
          // this is a new entry in the route table
          larDestType newEntry;
          newEntry.x = larhdr->destinationX();
          newEntry.y = larhdr->destinationY();
          newEntry.v = larhdr->destinationSpeed();
          newEntry.method = larhdr->forwardCode();
    
          newEntry.hops = entries;
          for (k=0; k<entries; k++)
          {
            newEntry.route[k] = larhdr->route_[myIndex-k];
          }
          newEntry.timeOfLastRequest = larhdr->requestSendTime();
          newEntry.timeOfVelocity = larhdr->speedTime();
          newEntry.requestPending = false;
          newEntry.noRoute = false;
          routeTable[dest] = newEntry;
          updatedRoute[dest] = true;
        } else {
          if (LARverbosity > 0)
          {
            cout << "(down) This route entry did exist, checking the time stamp." << endl;
            cout << "Route:  ";
            for (k=0; k<entries; k++)
            {
              cout << larhdr->route_[myIndex-k] << "  ";
            }
            cout << endl;
          }
          if (larhdr->speedTime() > destPtr->second.timeOfVelocity)
          {
            if (LARverbosity > 0)
            {
              cout << "(down) The time stamp is newer, so I'm using this one." << endl;
            }
            // this entry existed in the route table and we just need to update the information
            destPtr->second.x = larhdr->destinationX();
            destPtr->second.y = larhdr->destinationY();
            destPtr->second.v = larhdr->destinationSpeed();
            destPtr->second.method = larhdr->forwardCode();
  
            destPtr->second.hops = entries;
            for (k=0; k<entries; k++)
            {
              destPtr->second.route[k] = larhdr->route_[myIndex-k];
            }
            destPtr->second.timeOfLastRequest = larhdr->requestSendTime();
            destPtr->second.timeOfVelocity = larhdr->speedTime();
            destPtr->second.requestPending = false;
            destPtr->second.noRoute = false;
            updatedRoute[dest] = true;
          } else {
            if (LARverbosity > 0)
            {
              //cout << "(down) The time stamp is NOT newer, so re-setting the requestPending flag." << endl;
              cout << "(down) The time stamp is NOT newer." << endl;
            }
            //destPtr->second.requestPending = false;
            //destPtr->second.noRoute = true;
            //updated = false;
          }
        }
      }
    }
  }

  return;
}

// ***********************************************************************
//                            sendRouteReply
// ***********************************************************************
void LARAgent::sendRouteReply(Packet *p)
{
  // Access the common header for the received packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(p);
  struct hdr_cmn* hdrcmn = hdr_cmn::access(p);

  // old way to do this
  //hdr_cmn* hdrcmn = (hdr_cmn*)p->access(off_cmn_);
  // Access the LAR header for the received packet:
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  nsaddr_t dest = larhdr->destinationID();
  map<int, larDestType, less<int> >::iterator  destPtr;
  destPtr = routeTable.find(dest);
  if (destPtr->second.timeOfVelocity > larhdr->speedTime())
  {
    if (larhdr->hops() <= 1)
    {
      oneHopRtReqRx++;
    }
    // I need to reply with a route reply
    // Create a new packet
    Packet* newpkt = allocpkt();
    // Access the headers for the new packet:
    // new way to do this (2.1b7 and later)
    struct hdr_lar* newlarhdr = hdr_lar::access(newpkt);
    struct hdr_cmn* newhdrcmn = hdr_cmn::access(newpkt);

    // old way to do this
    //hdr_lar* newlarhdr = (hdr_lar*)newpkt->access(off_lar_);
    //hdr_cmn* newhdrcmn = (hdr_cmn*)newpkt->access(off_cmn_);
  
    newlarhdr->larCode_ = 'A';
    newlarhdr->forwardCode_ = larhdr->forwardCode();
    newlarhdr->sourceX_ = larhdr->sourceX_;
    newlarhdr->sourceY_ = larhdr->sourceY_;
    newlarhdr->lastHopX_ = node->X();
    newlarhdr->lastHopY_ = node->Y();
    newlarhdr->destinationX_ = destPtr->second.x;
    newlarhdr->destinationY_ = destPtr->second.y;
    newlarhdr->destinationID_ = larhdr->route_[0];
    newlarhdr->sendTime_ = Scheduler::instance().clock();
    newlarhdr->newPkt_ = true;
    newlarhdr->destinationSpeed_ = destPtr->second.v;
    newlarhdr->speedTime_ = destPtr->second.timeOfVelocity;

    // first, check to see if the route I have for the required destination
    // contains the requesting node
    int i;
    int srcIndex = -5;
    nsaddr_t srcID = larhdr->route_[larhdr->hops()-1];
    for (i=0; i<destPtr->second.hops; i++)
    {
      if (destPtr->second.route[i] == larhdr->route_[0])
      {
        srcIndex = i;
        break;
      }
    }
    //if (i==destPtr->second.hops)
    if (srcIndex == -5)
    {
      // the requesting node didn't exist in the route
      newlarhdr->hops() = destPtr->second.hops + larhdr->hops();
      int j;
      // puts the route that the join request took in the route
      for (j=0; j<larhdr->hops(); j++)
      {
        newlarhdr->route_[j] = larhdr->route_[j];
      }
      // puts the route from the route table on the end
      for (j=0; j<destPtr->second.hops; j++)
      {
        newlarhdr->route_[j+larhdr->hops()] = destPtr->second.route[j];
      }
    } else {
      int j=0;
      // the requesting node did exist in the route
      newlarhdr->hops() = destPtr->second.hops - srcIndex;
      for (i=srcIndex; i<destPtr->second.hops; i++, j++)
      {
        newlarhdr->route_[j] = destPtr->second.route[i];
      }
    }

    newlarhdr->requestID_ = hdrcmn->uid_;
    newlarhdr->requestSendTime_ = larhdr->requestSendTime();
    newlarhdr->dataLength_ = 0;
    if (LARverbosity > 0)
    {
      cout << "Sending a Route Reply, ID:  " << newhdrcmn->uid_ 
           << " to " << srcID << endl;
    }
    //this->broadcastPacket(newpkt);
    // Don't want to rebroadcast, want send it to my "upstream" neighbor

    this->sendPacket(newpkt, srcID);
  }
  return;
}

// ***********************************************************************
//                            sendNewData
// ***********************************************************************
void LARAgent::sendNewData(Packet *p, nsaddr_t destID)
{
  // Access the common header for the received packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(p);
  struct hdr_cmn* hdrcmn = hdr_cmn::access(p);

  // old way to do this
  //hdr_cmn* hdrcmn = (hdr_cmn*)p->access(off_cmn_);

  // Access the LAR header for the received packet:
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  node->update_position();

  map<nsaddr_t, larDestType, less<int> >::iterator  destPtr;
  destPtr = routeTable.find(destID);
  if (destPtr == routeTable.end())
  {
    // there is no entry for this destination in the route table
    // we need to create a routing table entry with the requestPending
    // field set, perform a route request, and put this packet on the
    // pending list

    // this is a new entry in the route table
    larDestType newEntry;
    newEntry.x = -1.0;
    newEntry.y = -1.0;
    newEntry.v = -1.0;
    newEntry.method = larhdr->forwardCode();
    newEntry.hops = 0;
    for (int k=0; k<maxRouteLength; k++)
    {
      newEntry.route[k] = -1;
    }
    newEntry.timeOfLastRequest = Scheduler::instance().clock();
    newEntry.timeOfVelocity = -1.0;
    newEntry.requestPending = true;
    newEntry.noRoute = true;
    routeTable[destID] = newEntry;
    updatedRoute[destID] = true;

    // send it out
    if (LARverbosity > 0)
    {
      cout << "****************************************************" << endl;
      cout << "Sending lar route request from:  " << this->addr()
           << " at [" << node->X() << "," << node->Y() << "]" << endl;
      cout << "Time:        " << Scheduler::instance().clock() << endl;
    }
    this->sendInitialRouteRequest(destID, &newEntry);

    // put the data packet on the pending list
    this->addToPendingPackets(p);
  } else if (destPtr->second.requestPending) {
    // there is already a route entry for this destination, but we need to check and see if
    // there is a pending request
    if (LARverbosity > 0)
    {
      cout << "****************************************************" << endl;
      cout << "There is already a pending route request for this destination:  " << destID << endl;
      cout << "Putting the data packet on the pending packet list.  Packet ID:  " << hdrcmn->uid_ << endl;
      cout << "Time:        " << Scheduler::instance().clock() << endl;
    }
    // put the data packet on the pending list
    this->addToPendingPackets(p);

  } else if (destPtr->second.noRoute) {
    // there is a routing entry, but I still haven't been able to find a route, so I need
    // to initiate another route request
    destPtr->second.requestPending = true;

    // send it out
    if (LARverbosity > 0)
    {
      cout << "****************************************************" << endl;
      cout << "There is a route entry, but no route..." << endl;
      cout << "Sending lar route request from:  " << this->addr()
           << " at [" << node->X() << "," << node->Y() << "]" << endl;
      cout << "Time:        " << Scheduler::instance().clock() << endl;
    }
    this->sendInitialRouteRequest(destID, &(destPtr->second));

    // put the data packet on the pending list
    this->addToPendingPackets(p);
  } else {
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// I may want to check the route table time stamp against the route time stamp
// in the header here, but I'm not sure what to do if the route I have is older,
// or if it actually matters...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // we can fill in the rest of the header fields from the routing
    // table and send this packet
    larhdr->destinationX_ = destPtr->second.x;
    larhdr->destinationY_ = destPtr->second.y;
    larhdr->destinationSpeed_ = destPtr->second.v;
    larhdr->speedTime_ = destPtr->second.timeOfVelocity;
    larhdr->requestSendTime_ = destPtr->second.timeOfLastRequest;
    larhdr->hops_ = destPtr->second.hops;
    for (int k=0; k<destPtr->second.hops; k++)
    {
      larhdr->route_[k] = destPtr->second.route[k];
    }
    // send it out
    if (LARverbosity > 0)
    {
      cout << "****************************************************" << endl;
      cout << "Sending lar data from:  " << this->addr()
           << " at [" << node->X() << "," << node->Y() << "]" << endl;
      cout << "Time:        " << Scheduler::instance().clock() << endl;
      cout << "Data size:   " << larhdr->dataLength() << endl;
      cout << "Total size:  " << larhdr->size() << endl;
    }
    this->sendPacket(p, destPtr->second.route[1]);
  }
  if (LARverbosity > 0)
  {
    cout << "****************************************************" << endl << endl;
  }
  return;
}

// ***********************************************************************
//                            routeRequest
// ***********************************************************************
void LARAgent::routeRequest(Packet *p)
{
  // Access the LAR header for the received packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(p);

  // old way to do this
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  // The destination for the route request is the last one in the source route
  nsaddr_t dest = larhdr->route_[larhdr->hops()-1];

  map<nsaddr_t, larDestType, less<int> >::iterator  destPtr;
  destPtr = routeTable.find(dest);
  if (destPtr == routeTable.end())
  {
    // I think this shouldn't happen, but I put it in just in case...
    // this is a new entry in the route table
    larDestType newEntry;
    newEntry.x = -1.0;
    newEntry.y = -1.0;
    newEntry.v = -1.0;
    newEntry.method = larhdr->forwardCode();
    newEntry.hops = 0;
    for (int k=0; k<maxRouteLength; k++)
    {
      newEntry.route[k] = -1;
    }
    newEntry.timeOfLastRequest = Scheduler::instance().clock();
    newEntry.timeOfVelocity = -1.0;
    newEntry.requestPending = true;
    newEntry.noRoute = true;
    routeTable[dest] = newEntry;
    updatedRoute[dest] = true;

    this->sendInitialRouteRequest(dest, &(newEntry));
  } else {
    if (LARverbosity > 0)
    {
      cout << "This route error is from a data packet I generated." << endl;
      cout << "Creating a route request to refresh the route." << endl;
    }
    // switch the routing table entry to pending, and set the noRoute flag
    destPtr->second.requestPending = true;
    destPtr->second.noRoute = true;

    // for now, the data packet that generated the route error is lost.
    // another option would be to return it with/after the route error and I
    // could put it in the pending packet list here

    this->sendRouteRequest(dest, larhdr->forwardCode(), &(destPtr->second), useRingZeroSearch);
  } // else of if (destPtr == routeTable.end())

  //Packet::free(p);
  return;
}

// ***********************************************************************
//                            subRoute
// ***********************************************************************
bool LARAgent::subRoute(larDestType *rt, Packet *p)
{
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(p);

  // old way to do this
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);
  int i, j;
  bool within = false;

  if (larhdr->hops() < rt->hops)
  {
    return false;
  }
  for (i=0; i<larhdr->hops(); i++)
  {
    for (j=0; j<rt->hops; j++)
    {
      if (rt->route[j] != larhdr->route_[i+j])
      {
        break;
      }
    }
    if (j == rt->hops)
    {
      return true;
    }
  }

  return within;
}

// ***********************************************************************
//                            routeError
// ***********************************************************************
void LARAgent::routeError(Packet *p)
{
  if (LARverbosity > 0)
  {
    cout << "Route Error Detected!" << endl;
    cout << "at node:  " << this->addr() << endl;
  }
  // this is called by the route error callback and should create a route
  // error message and send it upstream to the originator of the data packet
  // Access the common header for the received packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(p);
  struct hdr_cmn* hdrcmn = hdr_cmn::access(p);

  // old way to do this
  //hdr_cmn* hdrcmn = (hdr_cmn*)p->access(off_cmn_);

  // Access the LAR header for the received packet:
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  node->update_position();

  if (this->addr() == larhdr->route_[0])
  {
    if (LARverbosity > 0)
    {
      cout << "This is a first hop error.  Sending a route request" << endl;
    }
    // this is a first hop route error and means I should initiate a route request

    //cerr << "I just got a routeError callback and I originated the packet!" << endl;
    //cerr << "Packet ID:  " << hdrcmn->uid_ << endl;
    srcRtErrors++;
    // since this is a first hop route error, I can put the data packet on the pending list
    this->addToPendingPackets(p);
    routeRequest(p);

    return;
  } else {
    if (useIntermediateRouteRepair)
    {
      if (LARverbosity > 0)
      {
        cout << "Performing intermediate route repair and sending a route error." << endl;
      }
      // Create a new packet
      Packet* dataPkt = allocpkt();
      // Access the LAR header for the new packet:
      // new way to do this (2.1b7 and later)
      struct hdr_lar* newlarhdr = hdr_lar::access(dataPkt);
      struct hdr_cmn* hdrcmn = hdr_cmn::access(dataPkt);

      // old way to do this
      //hdr_lar* newlarhdr = (hdr_lar*)dataPkt->access(off_lar_);

      //hdr_cmn* hdrcmn = (hdr_cmn*)dataPkt->access(off_cmn_);
      hdrcmn->xmit_failure_ = larRouteErrorCallback;
      hdrcmn->xmit_failure_data_ = (void *) this;

      // Store the current time in the 'send_time' field
      newlarhdr->sendTime_ = larhdr->sendTime();

      newlarhdr->larCode_ = 'D';
      newlarhdr->forwardCode_ = larhdr->forwardCode();
      //cout << "forwardCode = " << larhdr->forwardCode() << endl;
      newlarhdr->sourceX_ = larhdr->sourceX();
      newlarhdr->sourceY_ = larhdr->sourceY();
      newlarhdr->lastHopX_ = node->X();
      newlarhdr->lastHopY_ = node->Y();
      newlarhdr->destinationID_ = larhdr->destinationID();
      newlarhdr->dataLength_ = larhdr->dataLength();
      newlarhdr->newPkt_ = false;

      newlarhdr->requestID_ = 0;
      newlarhdr->requestSendTime_ = 0;
      if (dropIntermediatePacketsIfNoRoute)
      {
        map<nsaddr_t, larDestType, less<int> >::iterator  destPtr;
        destPtr = routeTable.find(larhdr->destinationID());
        // again, this depends on short circuit boolean...
/*
        if ((destPtr != routeTable.end()) && (!destPtr->second.requestPending) && \
            (!destPtr->second.noRoute) && (destPtr->second.timeOfVelocity > larhdr->speedTime()))
*/
        if ((destPtr != routeTable.end()) && (!subRoute(&(destPtr->second),p)))
        {
          if (LARverbosity > 0)
          {
            cout << "Found a new route at (" << this->addr() << ").  Adding it and forwarding the packet." << endl;
            cout << "\tOld Route:  " << flush;
            int q;
            for (q=0; q<larhdr->hops(); q++)
            {
              cout << larhdr->route_[q] << "  " << flush;
            }
            cout << endl << "\tMy Route:  " << flush;
            for (q=0; q<destPtr->second.hops; q++)
            {
              cout << destPtr->second.route[q] << "  " << flush;
            }
            cout << endl;
          }
          if (LARverbosity > 0)
          {
            cout << "\t\tNot a sub route!" << endl;
          }
          sendNewData(dataPkt, larhdr->destinationID());
        } else {
          if (LARverbosity > 0)
          {
            cout << "Tried to do intermediate route repair, but had to"
                 << " drop the packet because there is no route." << endl;
          }
          if (destPtr != routeTable.end())
          {
            if (LARverbosity > 0)
            {
              // I can set no route since I know my information is older...
              cout << "\t\tIs a sub route!" << endl;
              cout << "Setting the route to no route."  << endl;
            }
            destPtr->second.noRoute = true;
          }
          // not going to use the packet I created above
          Packet::free(dataPkt);
        }
      } else {
        if (LARverbosity > 0)
        {
          cout << "Don't care if there is a route or not, trying to save the packet" << endl;
          cout << "no matter what." << endl;
        }
        sendNewData(dataPkt, larhdr->destinationID());
      }
    } else {
      if (LARverbosity > 0)
      {
        cout << "Not doing intermediate route repair, dropping the data packet and sending a route error." << endl;
      }
    }
    // This is not a first hop route error, so I should send back a route error packet
    
    // Create a new route error packet
    Packet* errorPkt = allocpkt();
    // Access the LAR header for the new packet:
    // new way to do this (2.1b7 and later)
    struct hdr_lar* larErrHdr = hdr_lar::access(errorPkt);

    // old way to do this
    //hdr_lar* larErrHdr = (hdr_lar*)errorPkt->access(off_lar_);
  
    // Store the current time in the 'send_time' field
    larErrHdr->sendTime_ = Scheduler::instance().clock();
  
    larErrHdr->larCode_ = 'E';
    larErrHdr->forwardCode_ = larhdr->forwardCode_;
    larErrHdr->sourceX_ = node->X();
    larErrHdr->sourceY_ = node->Y();
    larErrHdr->lastHopX_ = node->X();
    larErrHdr->lastHopY_ = node->Y();
    larErrHdr->destinationX_ = larhdr->sourceX();
    larErrHdr->destinationY_ = larhdr->sourceY();
    larErrHdr->destinationID_ = larhdr->route_[0];
    larErrHdr->destinationSpeed_ = 0;
    larErrHdr->speedTime_ = 0;
    larErrHdr->hops() = larhdr->hops();
    larErrHdr->dataLength_ = 0;
    larErrHdr->newPkt_ = true;
  
    larErrHdr->requestID_ = hdrcmn->uid_;
    larErrHdr->requestSendTime_ = larhdr->requestSendTime_;
  
    // copy the source route into the route error packet route.  Just an
    // important note here, the route in the error packet is "backwards".
    // Instead of reversing the route, I use it from end to beginning...
    nsaddr_t dest = -5;
    if (LARverbosity > 0)
    {
      cout << "Route:  " << flush;
    }
    for (int k=0; k<larhdr->hops(); k++)
    {
      if (LARverbosity > 0)
      {
        cout << larhdr->route_[k] << "  " << flush;
      }
      larErrHdr->route_[k] = larhdr->route_[k];
      if (this->addr() == larhdr->route_[k])
      {
        larErrHdr->route_[k] = routeBreakPoint;
        dest = larhdr->route_[k-1];
      }
    }
    routeErrorsGen++;
    if (LARverbosity > 0)
    {
      cout << endl;
  
      cout << "Creating a route error and sending to:  " << dest << endl;
    }
    if (dest != -5)
    {
      this->sendPacket(errorPkt, dest);
    } else {
      cerr << "There should be an error packet send to the upstream neighbor," << endl
           << "but I can't find my address in the route." << endl;
      cerr << "I'm going to broadcast it instead." << endl;
      larErrHdr->forwardCode_ = 'F';
      this->broadcastPacket(errorPkt);
    }
    Packet::free(p);
  }

  return;
}

// ***********************************************************************
//                            sendInitialRouteRequest
// ***********************************************************************
int LARAgent::sendInitialRouteRequest(nsaddr_t id, larDestType *routeEntry)
{
  if (useRingZeroSearch)
  {
    oneHopRtReqTx++;
  } else {
    floodRtReqTx++;
    if (LARverbosity > 0)
    {
      cout << "In sendInitialRouteRequest, flooding the first route request." << endl;
    }
  }
  node->update_position();

  // add/find a timer for this destination that purges the pending packets if it times out
  map<nsaddr_t, LARRouteRequestTimer*, less<int> >::iterator timerPtr;
  timerPtr = timers.find(id);
  if (timerPtr == timers.end())
  {
    if (useRingZeroSearch)
    {
      timers[id] = new LARRouteRequestTimer (this, id, 1, routeEntry, 'F');
      timers[id]->sched(ringZeroWait);
    } else {
      timers[id] = new LARRouteRequestTimer (this, id, 0, routeEntry, 'F');
      timers[id]->sched(LARrouteRequestTimeout);
    }
  } else {
    timerPtr->second->setForwardMethod('F');
    timerPtr->second->setRouteInfo(routeEntry);
    if (useRingZeroSearch)
    {
      timerPtr->second->setTriesLeft(1);
      timerPtr->second->resched(ringZeroWait);
    } else {
      timerPtr->second->setTriesLeft(0);
      timerPtr->second->resched(LARrouteRequestTimeout);
    }
  }

  // Create a new route request packet
  Packet* requestPkt = allocpkt();
  // Access the LAR header for the new packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larReqHdr = hdr_lar::access(requestPkt);
  struct hdr_cmn* requestHdrCmn = hdr_cmn::access(requestPkt);

  // old way to do this
  //hdr_lar* larReqHdr = (hdr_lar*)requestPkt->access(off_lar_);
  larReqHdr->hops() = 0;
  //hdr_cmn* requestHdrCmn = (hdr_cmn*)requestPkt->access(off_cmn_);

  // Store the current time in the 'send_time' field
  larReqHdr->sendTime_ = Scheduler::instance().clock();

  larReqHdr->larCode_ = 'R';
  if (useRingZeroSearch)
  {
    larReqHdr->forwardCode_ = 'N';
  } else {
    // all initial requests must flood!!
    larReqHdr->forwardCode_ = 'F';
  }
  larReqHdr->sourceX_ = node->X();
  larReqHdr->sourceY_ = node->Y();
  larReqHdr->lastHopX_ = node->X();
  larReqHdr->lastHopY_ = node->Y();
  larReqHdr->destinationX_ = 0;
  larReqHdr->destinationY_ = 0;
  larReqHdr->destinationID_ = id;
  larReqHdr->destinationSpeed_ = 0;
  larReqHdr->speedTime_ = 0;
  larReqHdr->hops()++;
  larReqHdr->dataLength_ = 0;
  larReqHdr->newPkt_ = true;

  larReqHdr->requestID_ = requestHdrCmn->uid_;
  larReqHdr->requestSendTime_ = larReqHdr->sendTime_;

  larReqHdr->route_[0] = this->addr();
  for (int k=1; k<maxRouteLength; k++)
  {
    larReqHdr->route_[k] = -1;
  }

  this->broadcastPacket(requestPkt);

  return requestHdrCmn->uid_;
}

// ***********************************************************************
//                            sendRouteRequest
// ***********************************************************************
int LARAgent::sendRouteRequest(nsaddr_t id, char fwdCode, larDestType *routeEntry, bool ringZero)
{
  node->update_position();

  if (LARverbosity > 0)
  {
    cout << "In sendRouteRequest and fwdCode = " << fwdCode << endl;
  }
  map<nsaddr_t, LARRouteRequestTimer*, less<int> >::iterator timerPtr;
  timerPtr = timers.find(id);
  if ((fwdCode == 'B') || (fwdCode == 'S'))
  {
    // add a timer for this destination in case we have to flood the route request
    if (timerPtr == timers.end())
    {
      if (ringZero)
      {
        timers[id] = new LARRouteRequestTimer (this, id, 2, routeEntry, fwdCode);
        timers[id]->sched(ringZeroWait);
        oneHopRtReqTx++;
      } else {
        timers[id] = new LARRouteRequestTimer (this, id, 1, routeEntry, 'F');
        timers[id]->sched(LARrouteRequestTimeout);
        larRtReqTx++;
      }
    } else {
      timerPtr->second->setRouteInfo(routeEntry);
      if (ringZero)
      {
        timerPtr->second->setForwardMethod(fwdCode);
        timerPtr->second->setTriesLeft(2);
        timerPtr->second->resched(ringZeroWait);
        oneHopRtReqTx++;
      } else {
        timerPtr->second->setForwardMethod('F');
        timerPtr->second->setTriesLeft(1);
        timerPtr->second->resched(LARrouteRequestTimeout);
        larRtReqTx++;
      }
    }
  } else {
    // add a timer for this destination that purges the pending packets if it times out
    if (timerPtr == timers.end())
    {
      timers[id] = new LARRouteRequestTimer (this, id, 0, routeEntry, 'F');
      timers[id]->sched(LARrouteRequestTimeout);
    } else {
      timerPtr->second->setForwardMethod('F');
      timerPtr->second->setTriesLeft(0);
      timerPtr->second->setRouteInfo(routeEntry);
      timerPtr->second->resched(LARrouteRequestTimeout);
    }
    floodRtReqTx++;
  }

  // Create a new route request packet
  Packet* requestPkt = allocpkt();
  // Access the LAR header for the new packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larReqHdr = hdr_lar::access(requestPkt);
  struct hdr_cmn* requestHdrCmn = hdr_cmn::access(requestPkt);

  // old way to do this
  //hdr_lar* larReqHdr = (hdr_lar*)requestPkt->access(off_lar_);
  larReqHdr->hops() = 0;
  //hdr_cmn* requestHdrCmn = (hdr_cmn*)requestPkt->access(off_cmn_);

  // Store the current time in the 'send_time' field
  larReqHdr->sendTime_ = Scheduler::instance().clock();

  larReqHdr->larCode_ = 'R';
  if (ringZero)
  {
    larReqHdr->forwardCode_ = 'N';
  } else {
    larReqHdr->forwardCode_ = fwdCode;
  }
  larReqHdr->sourceX_ = node->X();
  larReqHdr->sourceY_ = node->Y();
  larReqHdr->lastHopX_ = node->X();
  larReqHdr->lastHopY_ = node->Y();
  larReqHdr->destinationX_ = routeEntry->x;
  larReqHdr->destinationY_ = routeEntry->y;
  larReqHdr->destinationID_ = id;
  larReqHdr->destinationSpeed_ = routeEntry->v;
  larReqHdr->speedTime_ = routeEntry->timeOfVelocity;
  larReqHdr->hops()++;
  larReqHdr->dataLength_ = 0;
  larReqHdr->newPkt_ = true;

  larReqHdr->requestID_ = requestHdrCmn->uid_;
  larReqHdr->requestSendTime_ = larReqHdr->sendTime_;

  larReqHdr->route_[0] = this->addr();
  for (int k=1; k<maxRouteLength; k++)
  {
    larReqHdr->route_[k] = -1;
  }

  larReqHdr->requestID_ = requestHdrCmn->uid_;
  larReqHdr->requestSendTime_ = larReqHdr->sendTime_;

  this->broadcastPacket(requestPkt);

  return requestHdrCmn->uid_;
}

// ***********************************************************************
//                            clearRequestPending
// ***********************************************************************
void LARAgent::clearRequestPending(nsaddr_t id)
{
  map<nsaddr_t, larDestType, less<int> >::iterator  destPtr;
  destPtr = routeTable.find(id);
  if (destPtr == routeTable.end())
  {
    cerr << "Trying to clear a request pending flag in the route table and can't" << endl
         << "find the corresponding routing entry for node:  " << id << endl;
  } else {
    destPtr->second.requestPending = false;
  }

  return;
}

// ***********************************************************************
//                            addToPendingPackets
// ***********************************************************************
void LARAgent::addToPendingPackets(Packet *p)
{
  int amountToDump = pendingPackets.size() - pendingPacketQueueLength + 1;
                     // the "magic" + 1 is to make room for the packet we
                     // need to add
  // if there are fewer packets on the queue than the max length, then
  // this number will be negative and the loop will never execute

  for (int i=0; i<amountToDump; i++)
  {
    pktOverflow++;
    if (LARverbosity > 0)
    {
      cout << "Dropping a packet due to send queue overflow." << endl;
    }
    pendingPackets.pop_front();
  }
  pendingPackets.push_back(p);

  return;
}

// ***********************************************************************
//                            checkPendingQueueForPackets
// ***********************************************************************
void LARAgent::checkPendingQueueForPackets(nsaddr_t id)
{
  bool packetsLeftInQueue = false;
  list<Packet*>::iterator pendingPtr;
  for (pendingPtr=pendingPackets.begin(); pendingPtr!=pendingPackets.end(); /* nothing */)
  {
    // new way to do this (2.1b7 and later)
    struct hdr_lar* pendinglarhdr = hdr_lar::access((*pendingPtr));

    // old way to do this
    //hdr_lar* pendinglarhdr = (hdr_lar*)(*pendingPtr)->access(off_lar_);
    if (pendinglarhdr->destinationID() == id)
    {
      packetsLeftInQueue = true;
      break;
    }
  }
  if (packetsLeftInQueue)
  {
    map<nsaddr_t, larDestType, less<int> >::iterator  destPtr;
    destPtr = routeTable.find(id);
    if (destPtr == routeTable.end())
    {
      // this is a new entry in the route table, and it shouldn't be at this point!
cerr << "There is a pending packet on the pending packet"
     << " queue without a route table entry! (addr=" 
     << this->addr() << ") (id=" << id << ")" << endl;
      larDestType newEntry;
      newEntry.x = -1.0;
      newEntry.y = -1.0;
      newEntry.v = -1.0;
      newEntry.method = 'F';
      newEntry.hops = 0;
      for (int k=0; k<maxRouteLength; k++)
      {
        newEntry.route[k] = -1;
      }
      newEntry.timeOfLastRequest = -1.0;
      newEntry.timeOfVelocity = -1.0;
      newEntry.requestPending = false;
      newEntry.noRoute = true;
      routeTable[id] = newEntry;
      destPtr = routeTable.find(id);
    }
    destPtr->second.method = 'F';
    destPtr->second.requestPending = false;
    destPtr->second.noRoute = true;
    updatedRoute[id] = true;

    // reschedule a timer to timeout and restart the route request process
    map<nsaddr_t, LARRouteRequestTimer*, less<int> >::iterator timerPtr;
    timerPtr = timers.find(id);
    if (timerPtr == timers.end())
    {
      // I really hope this doesn't happen, because it means that something
      // serious went wrong and there isn't already a timer for this destination
      //if (useRingZeroSearch)
      //{
        //timers[id] = new LARRouteRequestTimer (this, id, 1, routeEntry, 'F');
        //timers[id]->sched(ringZeroWait);
      //} else {
cerr << "There is a problem.  I'm trying to fall back to persitent route request" << endl
     << "mode, and there is not a timer for this node!  (addr=" 
     << this->addr() << ") (id=" << id << ")" << endl;
        timers[id] = new LARRouteRequestTimer (this, id, 1, &(destPtr->second), 'F');
        timers[id]->sched(LARroutePersistenceTimeout);
      //}
    } else {
      timerPtr->second->setForwardMethod('F');
      timerPtr->second->setRouteInfo(&(destPtr->second));
      //if (useRingZeroSearch)
      //{
        //timerPtr->second->setTriesLeft(1);
        //timerPtr->second->resched(ringZeroWait);
      //} else {
        timerPtr->second->setTriesLeft(1);
        timerPtr->second->resched(LARroutePersistenceTimeout);
      //}
    }
  }
  return;
}

// ***********************************************************************
//                            purgePendingPacketsFor
// ***********************************************************************
void LARAgent::purgePendingPacketsFor(nsaddr_t id)
{
  int i = 1;
  double time = Scheduler::instance().clock();

  list<Packet*>::iterator pendingPtr;
  for (pendingPtr=pendingPackets.begin(); pendingPtr!=pendingPackets.end(); /* nothing */)
  {
    // new way to do this (2.1b7 and later)
    struct hdr_lar* pendinglarhdr = hdr_lar::access((*pendingPtr));

    // old way to do this
    //hdr_lar* pendinglarhdr = (hdr_lar*)(*pendingPtr)->access(off_lar_);
    if (pendinglarhdr->destinationID() == id)
    {
      if (fabs(time - pendinglarhdr->sendTime()) > dropPendingPacketsAfter)
      {
        if (pendinglarhdr->newPkt())
        {
          dataDroppedSrc++;
        } else {
          dataDroppedInt++;
        }
        if (LARverbosity > 0)
        {
          // new way to do this (2.1b7 and later)
          struct hdr_cmn* hdrCmn = hdr_cmn::access((*pendingPtr));

          // old way to do this
          //hdr_cmn* hdrCmn = (hdr_cmn*)(*pendingPtr)->access(off_cmn_);
          cout << "Deleting pending packet " << i++ << "  ID:  " << hdrCmn->uid_ << endl;
        }
        pendingPackets.erase(pendingPtr++);
      } else {
        pendingPtr++;
      }
    } else {
      pendingPtr++;
    }
  }

  return;
}

// ***********************************************************************
//                            purgeAllPendingPackets
// ***********************************************************************
void LARAgent::purgeAllPendingPackets()
{
  int i = 1;
  list<Packet*>::iterator pendingPtr;
  for (pendingPtr=pendingPackets.begin(); pendingPtr!=pendingPackets.end(); /* nothing */)
  {
    // new way to do this (2.1b7 and later)
    struct hdr_lar* pendinglarhdr = hdr_lar::access((*pendingPtr));

    // old way to do this
    //hdr_lar* pendinglarhdr = (hdr_lar*)(*pendingPtr)->access(off_lar_);
    if (pendinglarhdr->newPkt())
    {
      dataDroppedSrc++;
    } else {
      dataDroppedInt++;
    }
    droppedAtEnd++;
    if (LARverbosity > 0)
    {
      // new way to do this (2.1b7 and later)
      struct hdr_cmn* hdrCmn = hdr_cmn::access((*pendingPtr));

      // old way to do this
      //hdr_cmn* hdrCmn = (hdr_cmn*)(*pendingPtr)->access(off_cmn_);
      cout << "Deleting pending packet " << i++ << "  ID:  " << hdrCmn->uid_ << endl;
    }
    pendingPackets.erase(pendingPtr++);
  }
  return;
}

// ***********************************************************************
//                            dupRcvdPacket
// ***********************************************************************
bool LARAgent::dupRcvdPacket(int id)
{
  // I may need to limit the size of this in the future.
  if (rcvdPackets.find(id) != rcvdPackets.end())
  {
    return true;
  } else {
    rcvdPackets.insert(id);
    return false;
  }
}

// ***********************************************************************
//                            dupSentPacket
// ***********************************************************************
bool LARAgent::dupSentPacket(int id)
{
  // I may need to limit the size of this in the future.
  if (sentPackets.find(id) != sentPackets.end())
  {
    return true;
  } else {
    sentPackets.insert(id);
    return false;
  }
}

// ***********************************************************************
//                            inForwardingRegion
// ***********************************************************************
bool LARAgent::inForwardingRegion(char fwdCode,
                                  double srcX, double srcY,
                                  double destX, double destY, 
                                  double destSpeed, double speedTime)
{
  node->update_position();
  if (fwdCode == 'F')
  {
//cout << "One " << endl;
    return true;
  } else if (fwdCode == 'N') {
    return false;
  } else if (fwdCode == 'B') {
//cout << "Two " << endl;
    double delta = fabs(Scheduler::instance().clock() - speedTime);
    double radius = destSpeed * delta;
    double llx, lly, urx, ury;

    if (srcX < destX)
    {
      llx = lar_min(srcX, (destX-radius));
      urx = destX + radius;
    } else {
      llx = destX - radius;
      urx = lar_max(srcX, (destX+radius));
    }
    if (srcY < destY)
    {
      lly = lar_min(srcY, (destY-radius));
      ury = destY + radius;
    } else {
      lly = destY - radius;
      ury = lar_max(srcY, (destY+radius));
    }

    if ((node->X() >= (llx-LARDelta)) && (node->X() <= (urx+LARDelta)) &&
        (node->Y() >= (lly-LARDelta)) && (node->Y() <= (ury+LARDelta)))
    {
      return true;
    } else {
      return false;
    }
  } else if (fwdCode == 'S') {
//cout << "Three " << endl;
    double lastHopDistSquared = (srcX-destX)*(srcX-destX) + (srcY-destY)*(srcY-destY);
    double myDistSquared = (node->X()-destX)*(node->X()-destX) + (node->Y()-destY)*(node->Y()-destY);
    //cout << "lastHopDistSquared = " << lastHopDistSquared << endl;
    //cout << "myDistSquared = " << myDistSquared << endl;
    //cout << "LARDelta = " << LARDelta << endl;
    return (myDistSquared <= (lastHopDistSquared + (LARDelta)*(LARDelta)));
  } else {
    cerr << "inForwardingRegion and the forward code is not set!" << endl;
    return false;
  }
}

// ***********************************************************************
//                            recordSentPacket
// ***********************************************************************
void LARAgent::recordSentPacket(int id)
{
  sentPackets.insert(id);
  return;
}
  
// ***********************************************************************
//                            recordRcvdPacket
// ***********************************************************************
void LARAgent::recordRcvdPacket(int id)
{
  rcvdPackets.insert(id);
  return;
}

void LARprintHeaders(Packet *p)
{
  // Access the LAR header for the new packet:
  // new way to do this (2.1b7 and later)
  struct hdr_cmn* cmnhdr = hdr_cmn::access(p);
  struct hdr_ip*  iphdr  = hdr_ip::access(p);
  struct hdr_mac* machdr = hdr_mac::access(p);

  // old way to do this
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  // Access the common header for the new packet:
  //hdr_cmn* cmnhdr = (hdr_cmn*)p->access(hdr_cmn::offset());

  // Access the IP header for the new packet:
  //hdr_ip* iphdr = (hdr_ip*)p->access(hdr_ip::offset());

  // Access the mac header for the new packet:
  //hdr_mac* machdr = (hdr_mac*)p->access(hdr_mac::offset());

  cout << "IP Header Information:" << endl;
  cout << "\tSrc Address:   " << iphdr->saddr() << endl;
  cout << "\tSrc Port:      " << iphdr->sport() << endl;
  cout << "\tDest Address:  " << iphdr->daddr() << endl;
  cout << "\tDest Port:     " << iphdr->dport() << endl;
  cout << "\tTTL:           " << iphdr->ttl() << endl;

  cout << endl << "Common Header Information:" << endl;
  cout << "\tPacket Type:   " << p_info().name(cmnhdr->ptype()) << endl;
//  cout << "\tPacket Type:   " << cmnhdr->ptype() << endl;
  cout << "\tSize:          " << cmnhdr->size() << endl;
  cout << "\tUID:           " << cmnhdr->uid() << endl;
  cout << "\terror:         " << cmnhdr->error() << endl;
  cout << "\ttimestamp:     " << cmnhdr->timestamp() << endl;
  cout << "\tinterface:     " << cmnhdr->iface() << endl;
  cout << "\tDirection:     " << cmnhdr->direction() << endl;
  // cout << "\tref count:     " << cmnhdr->ref_count() << endl;
  cout << "\tprev hop:      " << cmnhdr->prev_hop_ << endl;
  cout << "\tnext hop:      " << cmnhdr->next_hop() << endl;
  cout << "\taddress type:  " << cmnhdr->addr_type() << endl;
  cout << "\tlast hop:      " << cmnhdr->last_hop_ << endl;
  cout << "\tnum forwards:  " << cmnhdr->num_forwards() << endl;
  cout << "\topt forwards:  " << cmnhdr->opt_num_forwards() << endl;

  cout << endl << "MAC Header Information:" << endl;
  cout << "\tSrc Address:   " << machdr->macSA() << endl;
  cout << "\tDest Address:  " << machdr->macDA() << endl;

  return;
}

// ***********************************************************************
//                            sendPacket
// ***********************************************************************
void LARAgent::sendPacket(Packet *p, nsaddr_t to)
{
  // Access the common header for the new packet:
  // new way to do this (2.1b7 and later)
  struct hdr_cmn* cmnhdr = hdr_cmn::access(p);
  struct hdr_ip*  iphdr  = hdr_ip::access(p);
  struct hdr_lar* larhdr = hdr_lar::access(p);

  // old way to do this
  //hdr_cmn* cmnhdr = (hdr_cmn*)p->access(off_cmn_);

  // Access the IP header for the new packet:
  //hdr_ip* iphdr = (hdr_ip*)p->access(off_ip_);

  // Access the lar header for the new packet:
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  //LARprintHeaders(p);

  // Update performance tracking variables
  if (larhdr->larCode() == 'D')
  {
    if ((larhdr->route_[0] == this->addr()) && (larhdr->newPkt()))
    {
      // I originated this data packet and it is a new one not an
      // intermediate reforward
      dataTxPkts++;
      dataTxBytes += larhdr->dataLength();
    } else {
      // this is a data packet and I'm just forwarding it along because
      // I'm on the route
      dataTxPktsFwd++;
      dataTxBytesFwd += larhdr->dataLength();
    }
    // count all header bytes, even those on data as control overhead
    protocolTxBytes += larhdr->headerLength();
  } else {
    // all other packets being sent by this agent at this layer are
    // protocol packets, either route request, reply, or error
    protocolTxPkts++;
    protocolTxBytes += larhdr->headerLength();
  }

  // set all the necessary things for the common header
  cmnhdr->next_hop_ = to;
  cmnhdr->prev_hop_ = this->addr();
  //cmnhdr->next_hop_ = 1;
  //cmnhdr->iface() = iface_literal::ANY_IFACE;    // any interface
  cmnhdr->direction() = hdr_cmn::DOWN;    // hopefully send it out
  //cmnhdr->direction() = hdr_cmn::UP;

  // now the ip header stuff
  iphdr->saddr() = this->addr();
  iphdr->sport() = 254;
  iphdr->daddr() = to;
  //iphdr->daddr() = 1;
  iphdr->dport() = 254;
  //iphdr->dport() = 255;         // router port
  iphdr->ttl() = 32;

  //cout << endl << "After Modification" << endl;
  //LARprintHeaders(p);

  // Send the packet
  recordSentPacket(cmnhdr->uid_);
  //cout << "LAR:  Sending a packet..." << endl;

  // The next three lines should all be equivilant ways to send a packet,
  // I'm just trying them all to get the broadcast to go.
  //send(p, 0);
  //send(p, ll);
  //target_->recv(p, (Handler*) 0);
  //Scheduler::instance().schedule(target_,p,0);

  double jitter = 0.0;
  if (LARuseJitteronSend)
  {
    // this one is different and was taken from dsragent::sendOutBCastPkt
    // I had to add a little jitter because it turned out that neighboring nodes
    // where re-broadcasting in the simulator at "exactly" the same time and
    // killing each other's transmissions.
    jitter = ((double)rand()/(double)RAND_MAX) / larJitter;
  } else {
    jitter = 0.0;
  }

  if (LARverbosity > 0)
  {
    cout << "Scheduling the packet for delivery:  " << cmnhdr->uid_ 
         << " with jitter=" << jitter << endl;
    cout << "Really sending at:  " << Scheduler::instance().clock() + jitter << endl;
    cout << "Common header size:  " << cmnhdr->size_ << endl;
  }
  cmnhdr->size_ = larhdr->size();
  if (LARverbosity > 0)
  {
    cout << "Common header size:  " << cmnhdr->size_ << endl;
  }
  Scheduler::instance().schedule(ll, p, jitter);

  if (!ll)
  {
    cerr << "Crap, the link layer is NULL!!!!!" << endl;
  }

//cout << "LAR:  Done sending." << endl;

  return;
}

// ***********************************************************************
//                            broadcastPacket
// ***********************************************************************
void LARAgent::broadcastPacket(Packet *p)
{
  // Access the common header for the new packet:
  // new way to do this (2.1b7 and later)
  struct hdr_cmn* cmnhdr = hdr_cmn::access(p);
  struct hdr_ip*  iphdr  = hdr_ip::access(p);
  struct hdr_lar* larhdr = hdr_lar::access(p);

  // old way to do this
  //hdr_cmn* cmnhdr = (hdr_cmn*)p->access(off_cmn_);

  // Access the IP header for the new packet:
  //hdr_ip* iphdr = (hdr_ip*)p->access(off_ip_);

  // Access the lar header for the new packet:
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  //LARprintHeaders(p);

  // Update performance tracking variables
  if (larhdr->larCode() == 'D')
  {
    cerr << "broadcastPacket called for data!  LAR should never do this." << endl;
    if ((larhdr->route_[0] == this->addr()) && (larhdr->newPkt()))
    {
      // I originated this data packet
      dataTxPkts++;
      dataTxBytes += larhdr->dataLength();
    } else {
      // this is a data packet and I'm just forwarding it along because
      // I'm on the route
      dataTxPktsFwd++;
      dataTxBytesFwd += larhdr->dataLength();
    }
    // count all header bytes, even those on data as control overhead
    protocolTxBytes += larhdr->headerLength();
  } else {
    // all other packets being sent by this agent at this layer are
    // protocol packets, either route request, reply, or error
    protocolTxPkts++;
    protocolTxBytes += larhdr->headerLength();
  }

  // set all the necessary things for the common header
  cmnhdr->next_hop_ = IP_BROADCAST;  // broadcast
  cmnhdr->prev_hop_ = this->addr();
  //cmnhdr->next_hop_ = 1;
  //cmnhdr->iface() = iface_literal::ANY_IFACE;    // any interface
  cmnhdr->direction() = hdr_cmn::DOWN;    // hopefully send it out
  //cmnhdr->direction() = hdr_cmn::UP;

  // now the ip header stuff
  iphdr->saddr() = this->addr();
  iphdr->sport() = 254;
  iphdr->daddr() = IP_BROADCAST;
  //iphdr->daddr() = 1;
  iphdr->dport() = 254;
  //iphdr->dport() = 255;         // router port
  iphdr->ttl() = 32;

  //cout << endl << "After Modification" << endl;
  //LARprintHeaders(p);

  // Send the packet
  recordSentPacket(cmnhdr->uid_);
  //cout << "LAR:  Sending a packet..." << endl;

  // The next three lines should all be equivilant ways to send a packet,
  // I'm just trying them all to get the broadcast to go.
  //send(p, 0);
  //send(p, ll);
  //target_->recv(p, (Handler*) 0);
  //Scheduler::instance().schedule(target_,p,0);

  double jitter = 0.0;
  if (LARuseJitteronBroadcast)
  {
    // this one is different and was taken from dsragent::sendOutBCastPkt
    // I had to add a little jitter because it turned out that neighboring nodes
    // where re-broadcasting in the simulator at "exactly" the same time and
    // killing each other's transmissions.
    jitter = ((double)rand()/(double)RAND_MAX) / larJitter;
  } else {
    jitter = 0.0;
  }

  if (LARverbosity > 0)
  {
    cout << "Scheduling the packet for delivery:  " << cmnhdr->uid_ 
         << " with jitter=" << jitter << endl;
    cout << "Really sending at:  " << Scheduler::instance().clock() + jitter << endl;
    cout << "Common header size:  " << cmnhdr->size_ << endl;
  }
  cmnhdr->size_ = larhdr->size();
  if (LARverbosity > 0)
  {
    cout << "Common header size:  " << cmnhdr->size_ << endl;
  }
  Scheduler::instance().schedule(ll, p, jitter);

  if (!ll)
  {
    cerr << "Crap, the link layer is NULL!!!!!" << endl;
  }

//cout << "LAR:  Done sending." << endl;

  return;
}

// ***********************************************************************
//                            rebroadcastPacket
// ***********************************************************************
// same as broadcastPacket, but 
//      - we don't change the ip header source address,
//      - we have to set previous hop,
//      - we need to decrement the ttl
//      - and we have to set the direction to down

void LARAgent::rebroadcastPacket(Packet *p)
{
  // Access the common header for the new packet:
  // new way to do this (2.1b7 and later)
  struct hdr_cmn* cmnhdr = hdr_cmn::access(p);
  struct hdr_ip*  iphdr  = hdr_ip::access(p);
  struct hdr_lar* larhdr = hdr_lar::access(p);

  // old way to do this
  //hdr_cmn* cmnhdr = (hdr_cmn*)p->access(off_cmn_);

  // Access the IP header for the new packet:
  //hdr_ip* iphdr = (hdr_ip*)p->access(off_ip_);

  // Access the lar header for the new packet:
  //hdr_lar* larhdr = (hdr_lar*)p->access(off_lar_);

  //LARprintHeaders(p);

  // Update performance tracking variables
  if (larhdr->larCode() == 'D')
  {
    cerr << "rebroadcastPacket called for data!  LAR should never do this." << endl;
    if ((larhdr->route_[0] == this->addr()) && (larhdr->newPkt()))
    {
      // I originated this data packet
      dataTxPkts++;
      dataTxBytes += larhdr->dataLength();
    } else {
      // this is a data packet and I'm just forwarding it along because
      // I'm on the route
      dataTxPktsFwd++;
      dataTxBytesFwd += larhdr->dataLength();
    }
    // count all header bytes, even those on data as control overhead
    protocolTxBytes += larhdr->headerLength();
  } else {
    // all other packets being sent by this agent at this layer are
    // protocol packets, either route request, reply, or error
    protocolTxPkts++;
    protocolTxBytes += larhdr->headerLength();
  }

  // set all the necessary things for the common header
  cmnhdr->next_hop_ = IP_BROADCAST;  // broadcast
  cmnhdr->prev_hop_ = this->addr();
  //cmnhdr->next_hop_ = 1;
  //cmnhdr->iface() = iface_literal::ANY_IFACE;    // any interface
  cmnhdr->direction() = hdr_cmn::DOWN;    // hopefully send it out
  //cmnhdr->direction() = hdr_cmn::UP;

  // now the ip header stuff
  // don't reset this on a rebroadcast
  //iphdr->saddr() = this->addr();

  // this doesn't have to be done, (it should be ok from the original)
  // but just in case...
  iphdr->sport() = 254;
  iphdr->daddr() = IP_BROADCAST;
  //iphdr->daddr() = 1;
  iphdr->dport() = 254;
  //iphdr->dport() = 255;         // router port

  // this is a difference from the regular broadcast
  // it appears that this needs to be done
  // since this packet doesn't go through the routing agent, this is never
  // done anywhere but here
//cout << "ttl before decrement:  " << iphdr->ttl_ << endl;
  iphdr->ttl()--;

  //cout << endl << "After Modification" << endl;
  //LARprintHeaders(p);

  // Send the packet
  recordSentPacket(cmnhdr->uid_);
  //cout << "LAR:  Sending a packet..." << endl;

  // The next three lines should all be equivilant ways to send a packet,
  // I'm just trying them all to get the broadcast to go.
  //send(p, 0);
  //send(p, ll);
  //target_->recv(p, (Handler*) 0);
  //Scheduler::instance().schedule(target_,p,0);

  double jitter = 0.0;
  if (LARuseJitteronBroadcast)
  {
    // this one is different and was taken from dsragent::sendOutBCastPkt
    // I had to add a little jitter because it turned out that neighboring nodes
    // where re-broadcasting in the simulator at "exactly" the same time and
    // killing each other's transmissions.
    jitter = ((double)rand()/(double)RAND_MAX) / larJitter;
  } else {
    jitter = 0.0;
  }

  if (LARverbosity > 0)
  {
    cout << "Scheduling the packet for delivery:  " << cmnhdr->uid_ 
         << " with jitter=" << jitter << endl;
    cout << "Really sending at:  " << Scheduler::instance().clock() + jitter << endl;
    cout << "Common header size:  " << cmnhdr->size_ << endl;
  }
  cmnhdr->size_ = larhdr->size();
  if (LARverbosity > 0)
  {
    cout << "Common header size:  " << cmnhdr->size_ << endl;
  }
  Scheduler::instance().schedule(ll, p, jitter);

  if (!ll)
  {
    cerr << "Crap, the link layer is NULL!!!!!" << endl;
  }

//cout << "LAR:  Done sending." << endl;

  return;
}

/*
    commented out until the full up promiscuous mode is working

// ***********************************************************************
//                            tap
// ***********************************************************************
void LARAgent::tap(const Packet *p)
{
  if (!usePromiscuousListening)
  {
    return;
  }

  // Access the common header for the received packet:
  // new way to do this (2.1b7 and later)
  struct hdr_lar* larhdr = hdr_lar::access(pkt);
  struct hdr_cmn* hdrcmn = hdr_cmn::access(pkt);
  struct hdr_ip* hdrip = hdr_ip::access(pkt);

  // ignore packets that will be passed up to my recv method anyway
  // (those addressed to me or the broadcast address)
  if ((hdrip->daddr() == this->addr()) || (hdrip->daddr() == IP_BROADCAST))
  {
    return;
  }

  // ignore if this is a duplicate packet
  if (this->dupRcvdPacket(hdrcmn->uid_) || this->dupSentPacket(hdrcmn->uid_))
  {
    return;
  } else {
    // This is redundant for now because dupRcvdPacket records the packet
    // as recieved, but I don't like that side effect and may change it
    // later.
    recordRcvdPacket(hdrcmn->uid_);
  }

  // now there must be some kind of call to a route cache maintenance routine

  return;
}

*/
