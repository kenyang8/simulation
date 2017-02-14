#ifndef __gpsr_seqtable_h__
#define __gpsr_seqtable_h__

#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>


#define INFINITY 0xff
#define RTF_DOWN 0
#define RTF_UP 1

#define REM_SEQ_COUNT 5000

class gpsr_RTEntry {
  friend class gpsr_RTable;
  friend class gpsr;

public:
  gpsr_RTEntry();
  gpsr_RTEntry(nsaddr_t src,u_int32_t seq);
  bool  isNewSeq(u_int32_t seq);  // old -> false, new->true
  void    addSeq(u_int32_t seq);  // add a seqno to seqno array(rt_seqnos)
  LIST_ENTRY(gpsr_RTEntry) rt_link;
protected:
  

  nsaddr_t src_;
//  u_int32_t seq_;

  u_int32_t   rt_seqnos[REM_SEQ_COUNT]; //seqno array
  u_int32_t         max_seqno;  //max seqno 
  u_int32_t         min_seqno;  //max seqno 
  u_int16_t   seq_it; // seqno's iterator
};


// The Routing Table
class gpsr_RTable {
  friend class gpsr;
public:
  gpsr_RTable() { LIST_INIT(&rthead); }
  void rt_delete(nsaddr_t id);
  gpsr_RTEntry* rt_lookup(nsaddr_t id);

  void rt_print();
  LIST_HEAD(, gpsr_RTEntry) rthead;
private:
  
  u_int32_t myseq_;
};

#endif