////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ant_pkt.h
/// This file defines packet structure that represents an ant for AntNet algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ant_pkt_h__
#define __ant_pkt_h__

#include <packet.h>
#include <list>
#include "antnet_common.h"

/// Forward ant identifier
#define FORWARD_ANT 0x01

/// Backward ant identifier
#define BACKWARD_ANT 0x02

/// Size of ant packet
#define ANT_SIZE 7

/// Macro to access ant header
#define HDR_ANT_PKT(p) hdr_ant_pkt::access(p)

////////////////////////////////////////////////////////////////////////////////////////////////
/// Represents memory of an ant
////////////////////////////////////////////////////////////////////////////////////////////////
struct memory{

	nsaddr_t node_addr;	// node address
	double trip_time;	// trip time to node
};

////////////////////////////////////////////////////////////////////////////////////////////////
/// Ant packet header
////////////////////////////////////////////////////////////////////////////////////////////////
struct hdr_ant_pkt {

        u_int8_t pkt_type_;				// Packet Type (forward/backward)
	nsaddr_t pkt_src_;				// address of source node (which originated the packet)
	nsaddr_t pkt_dst_;				// address of destination node
	u_int16_t pkt_len_;				// packet length
	u_int8_t pkt_seq_num_;				// packet sequence number
	double pkt_start_time_;				// packet start time
	struct memory pkt_memory_[MAX_NUM_NODES];	// packet's memory
	int pkt_mem_size_;				// size of memory
	static int offset_;				// offset for start of antnet header	
	
	inline nsaddr_t& pkt_src() {
		return pkt_src_;
	}
	inline nsaddr_t& pkt_dst() {
		return pkt_dst_;
	}
	inline u_int16_t& pkt_len() {
		return pkt_len_;
	}
	inline u_int8_t& pkt_seq_num() {
		return pkt_seq_num_;
	}
	inline double& pkt_start_time() {
		return pkt_start_time_;
	}
	inline int& pkt_mem_size() {
		return pkt_mem_size_;
	}
	inline u_int8_t& pkt_type() {
		return pkt_type_;
	}
	inline static int& offset() {
		return offset_;
	}	
	inline static hdr_ant_pkt* access(const Packet *p) {
		return (hdr_ant_pkt*)p->access(offset_);
	}
};

#endif
