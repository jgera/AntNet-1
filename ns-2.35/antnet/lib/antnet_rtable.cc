////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// antnet_rtable.cc
/// This file defines members of Antnet routing table class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "antnet_rtable.h"

double r_fact;		// reinforcement factor (value read from tcl script)
int N;			// Number of neighbors of a node
int NUM_NODES;		// number of nodes in topology

double old_ph_neighb[10],old_ph;

///////////////////////////////////////////////////
/// Method to add an entry in routing table
///////////////////////////////////////////////////
void antnet_rtable::add_entry_rtable(nsaddr_t dest, nsaddr_t next, double phvalue, int link_flag) {

	struct pheromone_rtable temp;		// create new pheromone structure
	temp.neighbor = next;			// set neighbor node (next hop for that destination) address
	temp.phvalue = phvalue;			// set pheromone value
	temp.link_flag = link_flag;		// set link status

	// note that the current nodes's routing table is considered
	rtable_t::iterator iterRt = rt_.find(dest);	// point to that destination

	// destination entry not in rtable, add new entry
	if (iterRt == rt_.end()) {
		pheromone_rtable_matrix temp_matrix;
		temp_matrix.push_back(temp);
		rt_[dest] = temp_matrix;
	}
	// destination entry exists in rtable, add neighbor entry
	else {
		pheromone_rtable_matrix *temp_matrix = &((*iterRt).second);
		temp_matrix->push_back(temp);
	}
}

///////////////////////////////////////////////////
/// Method to add an entry in soure table
///////////////////////////////////////////////////
void antnet_rtable::add_entry_stable(nsaddr_t source, nsaddr_t next, double phvalue, int link_flag) {

	struct pheromone_stable temp;		// create new pheromone structure
	temp.neighbor = next;			// set neighbor node (next hop for that destination) address
	temp.phvalue = phvalue;			// set pheromone value
	temp.link_flag = link_flag;		// set link status

	// note that the current nodes's routing table on link down is considered
	stable_t::iterator iterRt = st_.find(source);	// point to that source

	// source entry not in stable, add new entry
	if (iterRt == st_.end()) {
		pheromone_stable_matrix temp_matrix;
		temp_matrix.push_back(temp);
		st_[source] = temp_matrix;
	}
	// destination entry exists in stable, add neighbor entry
	else {
		pheromone_stable_matrix *temp_matrix = &((*iterRt).second);
		temp_matrix->push_back(temp);
	}
}

///////////////////////////////////////////////////
/// Method to print routing table into file
///////////////////////////////////////////////////
void antnet_rtable::print_rtable(char table) {

	FILE *fp;

	if (table == 'i')
		fp = fopen(file_itable,"a");
	else if (table == 'r')
		fp = fopen(file_rtable,"a");
	else if (table == 'd')
		fp = fopen(file_dtable,"a");
	else if (table == 'u')
		fp = fopen(file_utable,"a");

	for (rtable_t::iterator iter = rt_.begin(); iter != rt_.end(); iter++) {
		pheromone_rtable_matrix temp = (*iter).second;
		for (pheromone_rtable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++) {
			fprintf(fp,"\t\t%d\t%d\t%f\t%d\n", (*iter).first, (*iterPh).neighbor, (*iterPh).phvalue, (*iterPh).link_flag);
		}
	}
	fclose(fp);
}

///////////////////////////////////////////////////
/// Method to print source table into file
///////////////////////////////////////////////////
void antnet_rtable::print_stable(nsaddr_t addr) {

	FILE *fp = fopen(file_stable,"a");
	fprintf(fp,"\n\nSource table at node %d\n",addr);
	fprintf(fp,"\t\tSource\tNeighbor\tPhval\tLinkstat\n");
	for(stable_t::iterator iter = st_.begin(); iter != st_.end(); iter++) {
		pheromone_stable_matrix temp = (*iter).second;
		for (pheromone_stable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++)
			fprintf(fp,"\t\t%d\t%d\t\t%f\t%d\n", (*iter).first, (*iterPh).neighbor, (*iterPh).phvalue, (*iterPh).link_flag);
		
	}
	fclose(fp);
}

//////////////////////////////////////////////////////////////////////
/// Method to return a randomly chosen destination for a source node
/////////////////////////////////////////////////////////////////////
nsaddr_t antnet_rtable::calc_destination(nsaddr_t source) {

	int dest_chosen;
	do {
		dest_chosen = rnum->uniform(NUM_NODES);
	} while (dest_chosen == source);
	fprintf(stdout,"\nThe randomly chosen destination for the source %d is %d\n", dest_chosen, source);
	return dest_chosen;
}

//////////////////////////////////////////////////////////////////////
/// Method to return next hop node address
/////////////////////////////////////////////////////////////////////
nsaddr_t antnet_rtable::calc_next(nsaddr_t source, nsaddr_t dest, nsaddr_t parent) {

	nsaddr_t next;
	double thisph;
	double thisprob;
	double lrange = 0.0, urange = 0.0;
	double qtotal = 0.0;

	// find routing table entry for destination node
	rtable_t::iterator iter = rt_.find(dest);
	
	if (iter != rt_.end()) {
		pheromone_rtable_matrix temp = (*iter).second;
		for (pheromone_rtable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++) {
			next = (*iterPh).neighbor;
			Node *node1 = node1->get_node_by_address(source);
			Node *node2 = node2->get_node_by_address(next);
			int temp_len = get_queue_length(node1,node2);
			qtotal += temp_len;
		}

		if (qtotal == 0)
			qtotal = 1;

		// calculate probability range for parent link using lrange and urange
		for (pheromone_rtable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++) {
			next = (*iterPh).neighbor;
			thisph = (*iterPh).phvalue;
			Node *node1 = node1->get_node_by_address(source);
			Node *node2 = node2->get_node_by_address(next);
			int thisqueue = get_queue_length(node1,node2);
			
			thisprob = (thisph + ALPHA*(1 - thisqueue/qtotal)) / (1 + ALPHA*(N-1));

			// Move window by increasing urange value
			if (next == parent) {
				urange = lrange + (thisph);
				break;
			}
			lrange += (thisph);
		}

		if (urange == 0.0)
			urange = 1.0;
		
		// window has moved one circle (best path is to go via parent) - dead end, loopback
		if (lrange == 0.0 && urange == 1.0)
			return parent;

		// generate random probability value, out of range of parent link
		double tmp_double;
		do {
			tmp_double = rnum->uniform(1.0);
		}while (tmp_double >= lrange && tmp_double < urange);

		// find next hop node corresponding to this range of probability
		lrange = 0.0;
		urange = 0.0;
		for (pheromone_rtable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++) {
			next = (*iterPh).neighbor;
			thisph = (*iterPh).phvalue;
			Node *node1 = node1->get_node_by_address(source);
			Node *node2 = node2->get_node_by_address(next);
			int thisqueue = get_queue_length(node1,node2);

			thisprob = (thisph + ALPHA*(1 - thisqueue/qtotal)) / (1 + ALPHA*(N-1));
			urange += (thisph);

			// return neighbor if tmp_double is inside lrange-urange range and if link is up
			if (tmp_double >= lrange && tmp_double < urange && (*iterPh).link_flag) {
				fprintf(stdout,"\nThe chosen next hop for parent %d, source %d, dest %d, is to node %d, with tmp_double as %f\n",parent, source, dest, next, tmp_double);
				return next;
			}
			lrange = urange;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
/// Method to return feasibility of next hop for backward ant
///////////////////////////////////////////////////////////////////////////////////
bool antnet_rtable::check_next_hop(nsaddr_t source, nsaddr_t next) {

	stable_t::iterator iter = st_.find(source);
	if (iter != st_.end()) {

		pheromone_stable_matrix temp = (*iter).second;
		Node *nd = nd->get_node_by_address(source);

		for (pheromone_stable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++) {
			neighbor_list_node* nb = nd->neighbor_list_;
			while (nb != NULL) {
				if ((*iterPh).neighbor == next)
					if ((*iterPh).phvalue != 0.0)
						return true;
				nb = nb->next;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
/// Method to update routing table (increase/decrease ph value)
///////////////////////////////////////////////////////////////////////////////////
void antnet_rtable::update(nsaddr_t source, nsaddr_t dest, nsaddr_t next, bool stable_update) {
	
	rtable_t::iterator iterr = rt_.find(dest);
	pheromone_rtable_matrix *tempr;	
	if (iterr != rt_.end()) {
		tempr = &((*iterr).second);

		for (pheromone_rtable_matrix::iterator iterPhr = tempr->begin(); iterPhr != tempr->end(); iterPhr++) {
			double oldphr = (*iterPhr).phvalue;
			if ((*iterPhr).neighbor == next)
				(*iterPhr).phvalue = oldphr + r_fact*(1 - oldphr);	// increase ph value for link travelled by ant
			else
				(*iterPhr).phvalue = (1-r_fact)*oldphr;			// evaporate pheromone for other links
		}
	}

	if (stable_update) {
		stable_t::iterator iters = st_.find(source);
		pheromone_stable_matrix *temps;
		if(iters != st_.end()) {
			temps = &((*iters).second);
			for(pheromone_stable_matrix::iterator iterPhs = temps->begin(); iterPhs != temps->end(); iterPhs++) {
				double oldphs = (*iterPhs).phvalue;
				double& newphs = (*iterPhs).phvalue;
				if ((*iterPhs).neighbor == next)
					newphs = oldphs + r_fact*(1 - oldphs);		// increase ph value for link travelled by ant
				else
					newphs = (1-r_fact)*oldphs;			// increase ph value for link travelled by ant
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
/// Method to bring down a link and update routing table on link down
///////////////////////////////////////////////////////////////////////////////////
void antnet_rtable::link_down(nsaddr_t source, nsaddr_t neighb, nsaddr_t dest) {

	int y = 0;
	double temp_ph = 0.0;
	double num_nb = get_num_neighbors(source);

	// Read routing table on link down entry for source
	stable_t::iterator iter = st_.find(source);
	if(iter != st_.end()) 
	{
		// Updates for source node
		Node *nd = nd->get_node_by_address(source);
		neighbor_list_node* nb = nd->neighbor_list_;
		pheromone_stable_matrix temp = (*iter).second;

		for (pheromone_stable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++)	{
			nb = nd->neighbor_list_;

			while (nb != NULL) {
				if ((*iterPh).neighbor == neighb) {  
					old_ph = (*iterPh).phvalue;
					(*iterPh).link_flag = 0;

					// find phvalue to be distributed if it has neighbors	
					if (num_nb > 1)		
						temp_ph = (*iterPh).phvalue/(num_nb-1);
					else
						temp_ph = 0;
					(*iterPh).phvalue = 0.0;
				}
				else
					old_ph_neighb[y++] = (*iterPh).phvalue;

				if (y <= num_nb-1)
					break;
				nb = nb->next;
			}
		}
	
		remove_entry(dest);

		for(pheromone_stable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++) {
			if ((*iterPh).neighbor == neighb)
				(*iterPh).phvalue = 0.0;
			// add phvalue to neighbors
			else 
				(*iterPh).phvalue += temp_ph;
			add_entry_rtable(dest, (*iterPh).neighbor, (*iterPh).phvalue, (*iterPh).link_flag);
		}	
	}
}
		
///////////////////////////////////////////////////////////////////////////////////
/// Method to bring up a link and update routing table on link up
///////////////////////////////////////////////////////////////////////////////////
void antnet_rtable::link_up(nsaddr_t source, nsaddr_t neighb, nsaddr_t dest) {

	double num_nb = get_num_neighbors(source);

	// Read routing table on link down entry for source
	stable_t::iterator iter = st_.find(source);
	if(iter != st_.end()) {

		// Updates for source node
		Node *nd = nd->get_node_by_address(source);
		neighbor_list_node* nb = nd->neighbor_list_;
		pheromone_stable_matrix temp = (*iter).second;

		for (pheromone_stable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++) {
			nb = nd->neighbor_list_;
			
			int y=0;
			while (nb != NULL) {
				if ((*iterPh).neighbor == neighb) {  
					(*iterPh).phvalue = old_ph;
					(*iterPh).link_flag = 1;
				}
				else
					(*iterPh).phvalue = old_ph_neighb[y++];
				if (y <= num_nb-1)
					break;
				nb = nb->next;
			}
		}

		remove_entry(dest);
			
		for(pheromone_stable_matrix::iterator iterPh = temp.begin(); iterPh != temp.end(); iterPh++)
			add_entry_rtable(dest, (*iterPh).neighbor, (*iterPh).phvalue, (*iterPh).link_flag);
	}
}

///////////////////////////////////////////////////////////////////////////////////
/// Method to remove an entry from routing table
///////////////////////////////////////////////////////////////////////////////////
void antnet_rtable::remove_entry(nsaddr_t i) {
 	map<nsaddr_t, pheromone_rtable_matrix> ::iterator p=rt_.begin();
	map<nsaddr_t, pheromone_rtable_matrix> ::size_type a;
	a=rt_.erase(i);
}
