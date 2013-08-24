////////////////////////////////////////////////////////////////////////////////////////
/// antnet_common.cc
/// This file defines commonly used variables and functions
////////////////////////////////////////////////////////////////////////////////////////

#include "antnet_common.h"

//////////////////////////////////////////////////////
/// Method to return number of neighbors of a node
//////////////////////////////////////////////////////
int get_num_neighbors(nsaddr_t node_addr) {

	int count = 0;
	Node *nd = nd->get_node_by_address(node_addr);
	neighbor_list_node* nb = nd->neighbor_list_;
	while (nb != NULL) {
		count ++;
		nb = nb->next;
	}	
	return count;
}

//////////////////////////////////////////////////////////////
/// Method to return queue length of link between two nodes
/////////////////////////////////////////////////////////////
int get_queue_length(Node *node1, Node *node2) {

	Tcl& tcl = Tcl::instance();
	// get-drop-queue method implemented in ns-lib.tcl
	tcl.evalf("[Simulator instance] get-drop-queue %d %d", node1->nodeid(), node2->nodeid());
	DropTail *qa = (DropTail*)TclObject::lookup(tcl.result());
	int len = qa->getlength();
	return len;
}

//////////////////////////////////////////////////////////
///// Method to return feasibility to add another ant
////////////////////////////////////////////////////////////
bool get_ant_feasibility(int num_active_nodes, int num_active_ants) {
	if (num_active_ants < 150*num_active_nodes)
        	return true;

	fprintf(stdout,"\nForward generation skipped to prevent possible network congestion, currently %d ants active\n",num_active_ants);
	return false;
}
