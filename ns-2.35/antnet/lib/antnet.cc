////////////////////////////////////////////////
///  antnet.cc
/// This file defines members of Antnet class
////////////////////////////////////////////////

#include "antnet.h"
#include "address.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 

extern double r_fact;		// reinforcement factor
extern int N;			// number of neighbors of a node
extern int NUM_NODES;		// total number of nodes in the topology

///////////////////////////////////////////////////////////////////////////
/// Tcl binding for new packet: Ant
///////////////////////////////////////////////////////////////////////////
static class AntHeaderClass : public PacketHeaderClass {

 	public:
 		AntHeaderClass() : PacketHeaderClass("PacketHeader/Ant",sizeof(hdr_ant_pkt)) {
 			bind_offset(&hdr_ant_pkt::offset_);
 		}
} class_rtProtoAnt_hdr;

///////////////////////////////////////////////////////////////////////////
/// Tcl binding for new Agent: Antnet
///////////////////////////////////////////////////////////////////////////
static class AntnetClass : public TclClass {

	public:
		AntnetClass() : TclClass("Agent/Antnet") {}
		TclObject* create(int argc, const char*const* argv) {
			assert(argc == 5);
			return (new Antnet((nsaddr_t)Address::instance().str2addr(argv[4])));
		}
} class_rtProtoAntnet;
 
///////////////////////////////////////////////////////////////////////////
/// tcl binding for agent parameters - default values defined in ns-default.tcl
///////////////////////////////////////////////////////////////////////////
Antnet::Antnet(nsaddr_t id) : Agent(PT_ANT), ant_timer_(this), dmux_(0) {
	
	bind("num_nodes_", &num_nodes_);	// number of nodes in topology
	bind("r_factor_", &r_factor_);		// reinforcement factor
	bind("timer_ant_", &timer_ant_);	// timer for generation of forward ants
	  
	ra_addr_ = id;				// agent address 
	ant_seq_num_ = 1;			// initialize sequence number of ant packets to one

	NUM_NODES = num_nodes_;			// set number of nodes in topology (read from tcl script)
	r_fact = r_factor_;			// set reinforcement factor (read from tcl script)

	num_active_nodes=NUM_NODES;
	num_active_ants=0;
}

/////////////////////////////////////////////////////////////////
/// commands that the agent can handle
/////////////////////////////////////////////////////////////////
int Antnet::command(int argc, const char *const *argv) {

	if (argc == 2) {
		if (strcasecmp(argv[1], "start") == 0) {
			initialize_rtable();			// initialize routing table
			initialize_stable();			// initialize routing table on link down
			ant_timer_.resched(0.);			// schedule timer to begin ant generation now

			FILE *fp = fopen(file_itable,"a");
			fprintf(fp,"\n\nInitial routing table at node %d\n",addr());
			fprintf(fp,"\t\tDest\tNext\tPhval\t\tLinkstat\n");
			fclose(fp);

			rtable_.print_rtable('i');		// call method to print initial routing table
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "print_rtable") == 0) {
			FILE *fp = fopen(file_rtable,"a");
			fprintf(fp,"\n\nUpdated routing table at node %d\n",addr());
			fprintf(fp,"\t\tDest\tNext\tPhval\tLinkstat\n");
			fclose(fp);

			rtable_.print_rtable('r');		// call method to print routing table
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "print_dtable") == 0) {
			FILE *fp = fopen(file_dtable,"a");
			fprintf(fp,"\n\nRouting table on link down at node %d\n",addr());
			fprintf(fp,"\t\tDest\tNext\tPhval\tLinkstat\n");
			fclose(fp);

			rtable_.print_rtable('d');		// call method to print routing table on link down
			return TCL_OK;
		}
	    	else if (strcasecmp(argv[1], "print_utable") == 0) {
			FILE *fp = fopen(file_utable,"a");
			fprintf(fp,"\n\nRouting table on link up at node %d\n",addr());
			fprintf(fp,"\t\tDest\tNext\tPhval\tLinkstat\n");
			fclose(fp);

			rtable_.print_rtable('u');		// call method to print routing table on link up
			return TCL_OK;
		}
	  	else if (strcasecmp(argv[1], "print_stable") == 0) {	
			rtable_.print_stable(addr());		// call method to print source table
			return TCL_OK;
		}
		else if (strcasecmp(argv[1], "stop") == 0) {
			ant_timer_.cancel();			// cancel any scheduled timers
			return TCL_OK;
		}
	}
	else if (argc == 3) {
		// obtain corresponding dmux to carry packets
		if (strcmp(argv[1], "port-dmux") == 0) {
			dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
			if (dmux_ == 0) {
				fprintf(stderr, "%s: %s lookup of %s failed\n",__FILE__,argv[1],argv[2]);
				return TCL_ERROR;
 			}
			return TCL_OK;
 		}
		// obtain corresponding tracer
		else if (strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0) {
			logtarget_ = (Trace*)TclObject::lookup(argv[2]);
			if (logtarget_ == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
		else if (strcmp(argv[1],"link_break") == 0)
		{	
			int i=atoi(argv[2]);
			update_dtable(i);
		  	return TCL_OK;
		}
		else if (strcmp(argv[1],"link_up") == 0)
		{	
			int i=atoi(argv[2]);
			update_utable(i);
		  	return TCL_OK;
		}
	}
	// add node1 to neighbor list of node2 and vice-versa (we assume duplex link)
	else if (argc == 4) {
		if(strcmp(argv[1], "add-neighbor") == 0) {
			Node *node1 = (Node*)TclObject::lookup(argv[2]);
			Node *node2 = (Node*)TclObject::lookup(argv[3]);
			add_Neighbor(node1, node2);
			return TCL_OK;
		}
		
	}
	
	return Agent::command(argc, argv);			// Pass the command to the base class
}

/////////////////////////////////////////////////////////////////
/// Receive an antnet agent
/////////////////////////////////////////////////////////////////
void Antnet::recv(Packet *p, Handler *h) {

	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_ant_pkt *ah = HDR_ANT_PKT(p);

	// if sender is current agent
	if ((int)ih->saddr() == ra_addr()) {
		// if loop, drop the packet
		num_active_ants--;
		drop(p, DROP_RTR_ROUTE_LOOP);
	}
	else {
		// drop packets randomly
		int rnd_drop = num_active_ants*num_active_ants*17; 
		if ((rnd_drop%num_active_nodes) == 4) {
			num_active_ants--;
			drop(p, DROP_RTR_ROUTE_LOOP);
			fprintf(stdout,"\nOne ant packet lost while transmission\n");
		}

		// if recieved packet is an Ant packet, call method to handle it
		else if (ch->ptype() == PT_ANT)
			recv_ant_pkt(p);
		// if not Ant packet, drop
		else
			drop(p, DROP_MAC_PACKET_ERROR);
	}
}

/////////////////////////////////////////////////////////////////
/// Method to send a forward ant, called when ant timer expires
/////////////////////////////////////////////////////////////////
void Antnet::send_ant_pkt(nsaddr_t dest=-1) {

	nsaddr_t next;
	Packet* p = allocpkt();					// allocate new packet

	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_ant_pkt* ah = HDR_ANT_PKT(p);
	
	ah->pkt_type() = FORWARD_ANT;				// set ant type as FORWARD ant
	ah->pkt_src() = addr();					// source address
	ah->pkt_len() = ANT_SIZE;				// length of ant header
	ah->pkt_seq_num() = ant_seq_num_++;			// sequence number
	ah->pkt_start_time() = CURRENT_TIME;			// packet generation time
	
	if (dest == -1)
		dest = rtable_.calc_destination(addr());	// generate random destination
	
	ah->pkt_dst() = dest;					// set packet destination
	ah->pkt_mem_size() = 0;					// initialize size of memory
	ah->pkt_memory_[0].node_addr = addr();			// add source node to memory
	ah->pkt_memory_[0].trip_time = 0.0;			// add trip time to this node to memory
	ah->pkt_mem_size()++;					// increment size of memory
		
	ch->ptype() = PT_ANT;					// set packet type as Ant
	ch->direction() = hdr_cmn::DOWN;			// forward ant
	ch->size() = IP_HDR_LEN + ah->pkt_len();		// packet header size
	ch->error() = 0;
	ch->addr_type() = NS_AF_INET;

	// generate next hop as per AntNet algorithm
	next = rtable_.calc_next(addr(), ah->pkt_dst(), addr());

	// if next hop same as this node, release packet
	if (next == addr()) {
		Packet::free(p);
		return;
	}

	ch->next_hop() = next;		// set next hop address in common header
	ih->saddr() = addr();		// set source address in ip header
	ih->daddr() = next;		// set destination address in ip header
	ih->ttl() = 2 * (NUM_NODES);	// set time-to-live
	
	fprintf(stdout,"\nOne forward ant created, sending ant packet from %d to dest %d with next hop %d\n", ah->pkt_src(), ah->pkt_dst(), ih->daddr());
	
	num_active_ants++;
	fprintf(stdout,"No. of active ants = %d\n", num_active_ants);

	target_->recv(p);	// send forward ant packet - call the target nodes's recv function
}

///////////////////////////////////////////////////////////////////
/// Method to handle recieved Ant packet at agent
///////////////////////////////////////////////////////////////////
void Antnet::recv_ant_pkt(Packet *p) {

	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ant_pkt* ah = HDR_ANT_PKT(p);
	
	assert(ih->sport() == RT_PORT);
	assert(ih->dport() == RT_PORT);

	// forward ant
	if (ch->direction() == hdr_cmn::DOWN) {
		// destination node, travel complete
		if (addr() == ah->pkt_dst()) {
			memorize(p);			// add this node to memory
			num_active_ants--;
			create_backward_ant_pkt(p);	// create backward ant
		}
		// not destination node
		else {
			memorize(p);		// add this node to memory
			forward_ant_pkt(p);	// send forward ant to next hop node as determined by AntNet algorithm
		}
	}

	// backward ant
	else if (ch->direction() == hdr_cmn::UP) {
		// destination node, travel complete
		if (addr() == ah->pkt_dst()) {
			update_rtable(p);	// update routing table
			num_active_ants--;
			Packet::free(p);	// release packet
			return;
		}
		// not destination node
		else {
			update_rtable(p);	// update routing table
			backward_ant_pkt(p);	// send backward ant to next hop node as determined by memory
		}
	}
}

///////////////////////////////////////////////////////////////////
/// Method to build meory of forward ant
///////////////////////////////////////////////////////////////////
void Antnet::memorize(Packet *p) {

	struct hdr_ant_pkt* tmp = HDR_ANT_PKT(p);		// ant header
	
	double time = CURRENT_TIME - tmp->pkt_start_time();	// trip time to this node
	
	// If node revisited, there is a loop, remove loop and corresponding memory
	for (int i=0; i<tmp->pkt_mem_size(); i++)
		if (tmp->pkt_memory_[i].node_addr == addr()) {
			double t = time - tmp->pkt_memory_[i].trip_time;
			tmp->pkt_mem_size() = i+1;
			for (int j=0; j <= i; j++) {
				tmp->pkt_memory_[j].trip_time += t;
			}
			return;
		}
	
	// add current node to memory
	tmp->pkt_memory_[tmp->pkt_mem_size()].node_addr = addr();
	tmp->pkt_memory_[tmp->pkt_mem_size()].trip_time = time;
	tmp->pkt_mem_size() = tmp->pkt_mem_size()+1;

	fprintf(stdout,"\nAdding node %d to memory of pkt number %d\n", addr(), tmp->pkt_seq_num());
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Method to send forward ant packet to next hop node as determined by AntNet algorithm
////////////////////////////////////////////////////////////////////////////////////////
void Antnet::forward_ant_pkt(Packet *p) {

	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ant_pkt* ah = HDR_ANT_PKT(p);

	nsaddr_t parent = ih->saddr();		// parent node

	// find next hop node as per AntNet algorithm
	nsaddr_t next = rtable_.calc_next(addr(), ah->pkt_dst(), parent);

	// if next hop is this node or parent node, dead end, drop packet
	if (next == addr() || next == parent) {
		num_active_ants--;
		send_ant_pkt(ah->pkt_dst());	// auto generate one forward ant for forward ant loss
		drop(p, DROP_RTR_NO_ROUTE);
		fprintf(stdout,"\nOne forward ant auto generated\n");
		return;
	}

        fprintf(stdout,"\nFowarding forward ant from %d to %d", addr(), next);

	ch->next_hop() = next;		// set next hop node address in common header
	ih->saddr() = addr();		// set source address in ip header
	ih->daddr() = next;		// set destination address in ip header

	target_->recv(p);		// send forward ant packet - call the target nodes's recv function
}

//////////////////////////////////////////////////////////////////////////
/// Method to create backward ant packet, called when forward ant reaches destination node
//////////////////////////////////////////////////////////////////////////
void Antnet::create_backward_ant_pkt(Packet *p) {

	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ant_pkt* ah = HDR_ANT_PKT(p);
	
	// swap source and destination address
	nsaddr_t temp = ah->pkt_src();
	ah->pkt_src() = ah->pkt_dst();
	ah->pkt_dst() = temp;
	
	// retrieve last second entry in memory (last entry is this node)
	int index = ah->pkt_mem_size() - 2;

	ch->direction() = hdr_cmn::UP;				// chnge direction to backward Ant
	ch->ptype() = PT_ANT;					// set packet type as Ant
	ch->next_hop() = ah->pkt_memory_[index].node_addr;	// next hop as determined from memory

	ih->saddr() = addr();					// source address
	ih->daddr() = ch->next_hop();				// destination address
	
	fprintf(stdout,"\nOne backward ant created, sending ant packet from %d to %d with next hop %d\n", addr(), ah->pkt_src(), ah->pkt_dst(), ih->daddr());

	num_active_ants++;
	fprintf(stdout,"No. of active ants = %d\n", num_active_ants);

	target_->recv(p);					// send backward ant packet - call the target nodes's recv function
}

//////////////////////////////////////////////////////////////////////////////////
/// Method to send backward ant packet to next hop node, called when agent recieves a backward ant
//////////////////////////////////////////////////////////////////////////////////
void Antnet::backward_ant_pkt(Packet *p) {

	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ant_pkt* ah = HDR_ANT_PKT(p);
	
	// find node previous to this node in memory
	int index;
	for (int i = ah->pkt_mem_size()-1; i >= 0; i--)
		if (ah->pkt_memory_[i].node_addr == addr()) {
			index = i-1;
			break;
		}

	nsaddr_t next = ah->pkt_memory_[index].node_addr;
	ch->next_hop() = next;	
	ch->direction() = hdr_cmn::UP;			// backward ant
	ch->ptype() = PT_ANT;				// packet type = Ant
	ih->saddr() = addr();				// source address
	ih->daddr() = ch->next_hop();			// destination address

	if (!rtable_.check_next_hop(addr(), next)) {
		num_active_ants--;

		// auto generate two forward ants for backward ant loss
		send_ant_pkt(ah->pkt_src());
		send_ant_pkt(ah->pkt_dst());

		fprintf(stdout,"\nTwo forward ants auto generated\n");
		drop(p, DROP_RTR_NO_ROUTE);
		return;
	}

        fprintf(stdout,"\nFowarding backward ant from %d to %d", addr(), ch->next_hop());
	
	target_->recv(p);
}

//////////////////////////////////////////////////////////
/// Method to add neighbors of a node we assume duplex links
//////////////////////////////////////////////////////////
void Antnet::add_Neighbor(Node *n1, Node *n2) {
	n1->addNeighbor(n2);
	n2->addNeighbor(n1);
}

//////////////////////////////////////////////////////////
/// Method to reset Ant timer
//////////////////////////////////////////////////////////
void Antnet::reset_ant_timer() {
	ant_timer_.resched(timer_ant_);
}

//////////////////////////////////////////////////////////
/// Method to initialize routing table
//////////////////////////////////////////////////////////
void Antnet::initialize_rtable() {

	NUM_NODES = num_nodes_;		// set number of nodes in topology (read from tcl script)
	r_fact = r_factor_;		// set reinforcement factor (read from tcl script)

	nsaddr_t node_addr = addr();
	int num_nb = get_num_neighbors(node_addr);
	Node *nd = nd->get_node_by_address(addr());

	double best_phvalue = 0.000001*num_nb;
	double nobest_phvalue = (1.0-best_phvalue)/num_nb;
	double eq_phvalue = 1.0/num_nb;

	// add destination entry for each node in topology
	for (int i = 0; i < NUM_NODES; i++) {
		double min_phvalue = nobest_phvalue;

		if(addr() != i) {
			neighbor_list_node* nb = nd->neighbor_list_;	// read list of neighbors
			int best_neighb = nb->nodeid;			// let first neighbor be best for that destination
			int best_flag=0;				// 0 if there is no best neighbor

			if (node_addr < i) {				// find best neighbor greater than current node
				while (nb != NULL) {
					if (nb->nodeid == i) {
						best_neighb = nb->nodeid;
						best_flag=1;
						break;
					}
					if ((nb->nodeid <= i) && (nb->nodeid > node_addr)) {
						if (best_neighb < i)
							if (nb->nodeid > best_neighb)
								best_neighb = nb->nodeid;
						best_neighb = nb->nodeid;
						best_flag=1;
					}
					nb = nb->next;
				}
			}
			else {						// find best neighbor lesser than current node
				while (nb != NULL) {
					if (nb->nodeid == i) {
						best_neighb = nb->nodeid;
						best_flag=1;
						break;
					}
					if ((nb->nodeid >= i) && (nb->nodeid <= best_neighb) && (nb->nodeid < node_addr)){
						if (best_neighb > i)
							if (nb->nodeid > best_neighb)
								best_neighb = nb->nodeid;	
						best_neighb = nb->nodeid;
						best_flag=1;
					}
					nb = nb->next;
				}
			}
			nb = nd->neighbor_list_;

			if (best_flag == 0)				// no best neighbor, so equal pheromones to all
				min_phvalue=eq_phvalue;	

			while(nb != NULL) {
				int neighb = nb->nodeid;				// read node id of neighbor node
				double phvalue = min_phvalue;				// initialize min ph value to all neighbor links

				if ((nb->nodeid == best_neighb) && (best_flag != 0))
					phvalue += best_phvalue;			// increase phvalue for best neighbor

				int link_flag=1;					// link is up	
				rtable_.add_entry_rtable(i, neighb, phvalue,link_flag);	// add routing table entry
				
				nb = nb->next;						// iterate in neighbor list
			}
		}
	}
}

//////////////////////////////////////////////////////////
/// Method to initialize the source table
//////////////////////////////////////////////////////////
void Antnet::initialize_stable() {

	nsaddr_t node_addr = addr();
	Node *nd = nd->get_node_by_address(addr());
	neighbor_list_node* nb = nd->neighbor_list_;					// read list of neighbors
	int num_nb = get_num_neighbors(node_addr);
	
	while (nb != NULL){
		int neighb = nb->nodeid;
		double phvalue = 1.0/num_nb;						// assign equal pheromone values
		int link_flag=1;
		rtable_.add_entry_stable(node_addr, neighb, phvalue, link_flag);	// add routing table entry
		nb = nb->next;
	}
}

//////////////////////////////////////////////////////////
/// Method to update routing table
//////////////////////////////////////////////////////////
void Antnet::update_rtable(Packet *p) {
	
	int i;
	nsaddr_t dest, next;
	struct hdr_ant_pkt* ah = HDR_ANT_PKT(p);	// ant header
	
	// read node visited next to this node from memory, this is the nieghbor node for which routing table will be updated
	for (i=0; ah->pkt_memory_[i].node_addr != addr(); i++);
	i++;
	next = ah->pkt_memory_[i].node_addr;
		
	nsaddr_t node_addr = addr();
	N = get_num_neighbors(node_addr);
	
	// ph values in routing table is updated for all the destination nodes that are visited after the neighbor node
	for (int index = i; index < ah->pkt_mem_size(); index++) {
		// read destination node from memory
		dest = ah->pkt_memory_[index].node_addr;

		// update pheromone value for neighbor node and this destination node
		rtable_.update(addr(), dest, next, stable_update);		
	}
}

//////////////////////////////////////////////////////////
/// Method to break link on link down
//////////////////////////////////////////////////////////
void Antnet::update_dtable(nsaddr_t node_id) {

	nsaddr_t node_addr = addr();
	Node *nd = nd->get_node_by_address(addr());
	neighbor_list_node* nb = nd->neighbor_list_;				// read list of neighbors
	int num_nb = get_num_neighbors(node_addr);
	stable_update = false;
	
	for (int i = 0; i < NUM_NODES; i++)
		if (addr() != i) {
			nb = nd->neighbor_list_;
			
			while(nb != NULL) {
				if(nb->nodeid == node_id)
				  	rtable_.link_down(node_addr, nb->nodeid, i); // link down with neighb
				nb = nb->next;
			}
		}
}

//////////////////////////////////////////////////////////
/// Method to repair a previously down link
//////////////////////////////////////////////////////////
void Antnet::update_utable(nsaddr_t node_id) {

	nsaddr_t node_addr = addr();
	Node *nd = nd->get_node_by_address(addr());
	int num_nb = get_num_neighbors(node_addr);
	neighbor_list_node* nb = nd->neighbor_list_;				// read list of neighbors
		
	for(int i = 0; i < NUM_NODES; i++)
		if(addr() != i) {
			nb = nd->neighbor_list_;

			while(nb != NULL) {
				if(nb->nodeid == node_id)
					rtable_.link_up(node_addr, nb->nodeid, i); // link up with neighb
				nb = nb->next;
			}
		}
}

//////////////////////////////////////////////////////////
/// Method to handle Ant timer expire event
//////////////////////////////////////////////////////////
void Ant_timer::expire(Event *e) {
	if (get_ant_feasibility(num_active_nodes, num_active_ants))
		agent_->send_ant_pkt();		// generate forward ant

	agent_->reset_ant_timer();		// reschedule timer
}
