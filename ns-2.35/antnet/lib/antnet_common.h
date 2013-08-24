////////////////////////////////////////////////////////////////////////////////////////
/// antnet_common.h
/// This file declares commonly used variables and functions
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __antnet_common_h__
#define __antnet_common_h__

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

/// Macro to retrieve current simulator time 
#define CURRENT_TIME Scheduler::instance().clock()

/// Maximum number of nodes in topology
#define MAX_NUM_NODES 16

/// File names for various tables of AntNet
#define file_itable "itable.txt" // Initial routing table
#define file_rtable "rtable.txt" // Routing table
#define file_dtable "dtable.txt" // Routing table on link down
#define file_utable "utable.txt" // Routing table on link up
#define file_stable "stable.txt" // Source table

/// AntNet parameters
#define ALPHA 0.45

/// Method to return number of neighbors of a node
int get_num_neighbors(nsaddr_t);

/// Method to return queue length of a link between two nodes
int get_queue_length(Node *, Node *);

/// Method to return feasibility of new forward ant packet	
bool get_ant_feasibility(int, int);
 
#endif
