////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// antnet_rtable.h
/// This file defines Antnet rotuing table class and declares it members
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __antnet_rtable_h__
#define __antnet_rtable_h__

#include <trace.h>
#include <map>
#include <string>
#include <vector>
#include <classifier-port.h>
#include <random.h>

#include "ant_pkt.h"
#include "antnet_common.h"

////////////////////////////////////////////////////////////////////////////////////////////////
/// Represents an entry in routing tables - structure represents ph value corresponding to a neighbor node
////////////////////////////////////////////////////////////////////////////////////////////////
struct pheromone_rtable {

	nsaddr_t neighbor;	// address of neighbor node
	double phvalue;		// ph value corresponding to neighbor
	int link_flag;		// link status - 1 idicates link is up
};

/// vector of pheromone values (represents entry in routing table corresponding to a destination)
typedef std::vector<struct pheromone_rtable> pheromone_rtable_matrix;

/// Routing table
typedef std::map<nsaddr_t, pheromone_rtable_matrix> rtable_t;

////////////////////////////////////////////////////////////////////////////////////////////////
/// Represents an entry in routing tables on link down - structure represents ph value corresponding to a neighbor node
////////////////////////////////////////////////////////////////////////////////////////////////
struct pheromone_stable {

	nsaddr_t neighbor;	// address of neighbor node
	double phvalue;		// ph value corresponding to neighbor
	int link_flag;		// link status - 1 idicates link is up
};

/// vector of pheromone values (represents entry in routing table on link down corresponding to a destination)
typedef std::vector<struct pheromone_stable> pheromone_stable_matrix;

/// Routing table on link down
typedef std::map<nsaddr_t, pheromone_stable_matrix> stable_t;

/////////////////////////////////////////////////////////////
/// Class to implement routing table
/////////////////////////////////////////////////////////////
class antnet_rtable {

	RNG *rnum;		// random number generator
	rtable_t rt_;		// routing table
	stable_t st_;		// routing table on link down
	
	public:

		antnet_rtable() {
			rnum = new RNG((long int)CURRENT_TIME);
		}

		// Methods to add an entry in routing tables - Parameters: destination node, neighbor node, pheromone value
		void add_entry_rtable(nsaddr_t, nsaddr_t, double, int);
		void add_entry_stable(nsaddr_t, nsaddr_t, double, int);

		// Methods to print routing tables
		void print_rtable(char); 		// routing table
		void print_stable(nsaddr_t);		// source table

		// Returns destination node for given source node
		nsaddr_t calc_destination(nsaddr_t);

		// Returns next hop node for given source-destination pair - Parameters: source node, destination node, parent node
		nsaddr_t calc_next(nsaddr_t, nsaddr_t, nsaddr_t);

		// Returns feasibility of next hop for backward ant
		bool check_next_hop(nsaddr_t, nsaddr_t);

		// Updates an entry in routing table - Parameters: destination node, neighbor node
		void update(nsaddr_t, nsaddr_t, nsaddr_t, bool);

		// Updates routing tables on link down and up
		void link_down(nsaddr_t, nsaddr_t, nsaddr_t);
		void link_up(nsaddr_t, nsaddr_t, nsaddr_t);
		
		void remove_entry(nsaddr_t);
};

#endif
