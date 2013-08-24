////////////////////////////////////////////////
/// antnet.h
/// This file defines Antnet class and declares it members
////////////////////////////////////////////////

#ifndef __antnet_h__
#define __antnet_h__

#include <agent.h>
#include <node.h>
#include <packet.h>
#include <ip.h>
#include <trace.h>
#include <timer-handler.h>
#include <random.h>
#include <classifier-port.h>
#include <tools/rng.h>

#include "trace/cmu-trace.h"
#include "tools/queue-monitor.h"
#include "queue/drop-tail.h"

#include "ant_pkt.h"
#include "antnet_common.h"
#include "antnet_rtable.h"

#include <map>
#include <vector>
#include <list>

/// Variables used to limit number of ants
int num_active_nodes;
int num_active_ants;
bool stable_update=true;

class Antnet;	// forward declaration

////////////////////////////////////////////////////////////////////////////////////
/// Class to implement timer for interval between generation of forward ants
///////////////////////////////////////////////////////////////////////////////////
class Ant_timer: public TimerHandler {

	public:
		Ant_timer(Antnet* agent) : TimerHandler() {
			agent_ = agent;
		}

	protected:
		Antnet* agent_;
		virtual void expire(Event* e);
};

///////////////////////////////////////////////
/// Class to implement Antnet agent which implements AntNet algorithm
///////////////////////////////////////////////
class Antnet: public Agent {
	
	friend class Ant_timer;

	nsaddr_t ra_addr_;					// address of the agent
	antnet_rtable rtable_;					// instance of routing table class
	u_int8_t ant_seq_num_;					// sequence number for ant packets
	
	// parameters that can be set from tcl script - default values defined in ns-default.tcl
	double r_factor_;					// reinforcement factors
	double timer_ant_;					// interval between generation of forward ants
	int num_nodes_;						// total number ofnodes in topology
	
	protected:
		PortClassifier* dmux_;				// for passing packets to agent
		Trace* logtarget_;				// for logging
		Ant_timer ant_timer_;				// timer for generation of ants
		
		inline nsaddr_t& ra_addr() {
			return ra_addr_;
		}
		inline u_int8_t& ant_seq_num() {
			return ant_seq_num_;
		}

		void send_ant_pkt(nsaddr_t);			// generate forward ant
		void recv_ant_pkt(Packet *);			// recieve an ant packet
		void memorize(Packet *);			// add visited node to memory of forward ant
		void forward_ant_pkt(Packet *);			// send a forward ant to next hop as per AntNet algorithm
		void create_backward_ant_pkt(Packet*);		// generate backward ant
		void backward_ant_pkt(Packet *);		// send a backward ant to next hop as per AntNet algorithm
		
		void print_neighbors();				// print neighbors of a node
		void add_Neighbor(Node *, Node *);		// add two nodes to each other's neighbor list (assuming duplex link)
		void reset_ant_timer();				// reset ant timer

		void initialize_rtable();			// initialize routing table
		void initialize_stable();			// initialize source table

		void update_rtable(Packet*);			// update routing table
		void update_dtable(nsaddr_t); 			// update routing table on link down
		void update_utable(nsaddr_t); 			// update routing table on link up

	public:
		Antnet(nsaddr_t);				// constructor
		int command(int, const char*const*);		// interface for tcl commands
		void recv(Packet *, Handler *);			// method to handle packet recieve events at the Agent
};

#endif

