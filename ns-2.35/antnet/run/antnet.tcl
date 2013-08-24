# Tcl script for AntNet algorithm on an network topology of 12 nodes

# Set the number of nodes
set sz 12

# Create event Schedular
set ns [ new Simulator ]

# Open the Trace file
set tf [open antnet_trace.out w]
$ns trace-all $tf

# Open the nam trace file
set nf [open antnet_nam.nam w]
$ns namtrace-all $nf

# Create 12 nodes - from 0 to 11
for {set i 0} {$i < $sz} {incr i} {
	set n($i) [$ns node]
}

# Create 2 way links between the nodes
$ns duplex-link $n(0) $n(1) 512Mb 155ms DropTail
$ns duplex-link $n(3) $n(2) 512Mb 155ms DropTail
$ns duplex-link $n(2) $n(9) 512Mb 155ms DropTail
$ns duplex-link $n(2) $n(1) 512Mb 155ms DropTail
$ns duplex-link $n(2) $n(4) 512Mb 155ms DropTail
$ns duplex-link $n(9) $n(8) 512Mb 155ms DropTail
$ns duplex-link $n(9) $n(4) 512Mb 155ms DropTail
$ns duplex-link $n(11) $n(1) 512Mb 155ms DropTail
$ns duplex-link $n(1) $n(8) 512Mb 155ms DropTail
$ns duplex-link $n(1) $n(10) 512Mb 155ms DropTail
$ns duplex-link $n(1) $n(7) 512Mb 155ms DropTail
$ns duplex-link $n(8) $n(4) 512Mb 155ms DropTail
$ns duplex-link $n(8) $n(7) 512Mb 155ms DropTail
$ns duplex-link $n(8) $n(6) 512Mb 155ms DropTail
$ns duplex-link $n(4) $n(5) 512Mb 155ms DropTail
$ns duplex-link $n(7) $n(6) 512Mb 155ms DropTail
$ns duplex-link $n(6) $n(5) 512Mb 155ms DropTail

# Create Antnet agents, one for each of the nodes
for {set i 0} {$i < $sz} {incr i} {
	set   nn($i)  [ new Agent/Antnet $i]
}

# Attach each node with Antnet agent
for {set i 0} {$i < $sz} {incr i} {
	$ns attach-agent  $n($i)  $nn($i)
}

# Create connection between the nodes
$ns connect $nn(0) $nn(1)
$ns connect $nn(1) $nn(0)
$ns connect $nn(3) $nn(2)
$ns connect $nn(2) $nn(3)
$ns connect $nn(2) $nn(9)
$ns connect $nn(9) $nn(2)
$ns connect $nn(2) $nn(1)
$ns connect $nn(1) $nn(2)
$ns connect $nn(2) $nn(4)
$ns connect $nn(4) $nn(2)
$ns connect $nn(9) $nn(8)
$ns connect $nn(8) $nn(9)
$ns connect $nn(9) $nn(4)
$ns connect $nn(4) $nn(9)
$ns connect $nn(11) $nn(1)
$ns connect $nn(1) $nn(11)
$ns connect $nn(1) $nn(8)
$ns connect $nn(8) $nn(1)
$ns connect $nn(1) $nn(10)
$ns connect $nn(10) $nn(1)
$ns connect $nn(1) $nn(7)
$ns connect $nn(7) $nn(1)
$ns connect $nn(8) $nn(4)
$ns connect $nn(4) $nn(8)
$ns connect $nn(8) $nn(7)
$ns connect $nn(7) $nn(8)
$ns connect $nn(8) $nn(6)
$ns connect $nn(6) $nn(8)
$ns connect $nn(4) $nn(5)
$ns connect $nn(5) $nn(4)
$ns connect $nn(7) $nn(6)
$ns connect $nn(6) $nn(7)
$ns connect $nn(6) $nn(5)
$ns connect $nn(5) $nn(6)

# Add neighbors
$ns at now "$nn(0) add-neighbor $n(0) $n(1)"
$ns at now "$nn(0) add-neighbor $n(3) $n(2)"
$ns at now "$nn(0) add-neighbor $n(2) $n(9)"
$ns at now "$nn(0) add-neighbor $n(2) $n(1)"
$ns at now "$nn(0) add-neighbor $n(2) $n(4)"
$ns at now "$nn(0) add-neighbor $n(9) $n(8)"
$ns at now "$nn(0) add-neighbor $n(9) $n(4)"
$ns at now "$nn(0) add-neighbor $n(11) $n(1)"
$ns at now "$nn(0) add-neighbor $n(1) $n(8)"
$ns at now "$nn(0) add-neighbor $n(1) $n(10)"
$ns at now "$nn(0) add-neighbor $n(1) $n(7)"
$ns at now "$nn(0) add-neighbor $n(8) $n(4)"
$ns at now "$nn(0) add-neighbor $n(8) $n(7)"
$ns at now "$nn(0) add-neighbor $n(8) $n(6)"
$ns at now "$nn(0) add-neighbor $n(4) $n(5)"
$ns at now "$nn(0) add-neighbor $n(7) $n(6)"
$ns at now "$nn(0) add-neighbor $n(6) $n(5)"

# Set parameters and start time
for {set i 0} {$i < $sz} {incr i} {
	$nn($i) set num_nodes_ $sz
	$nn($i) set timer_ant_ 0.005
	$nn($i) set r_factor_ 0.05
	$ns  at  1.0  "$nn($i) start"
}

# Print the source table for each node
for {set i 0} {$i < $sz} {incr i} {
	$ns at 4.8 "$nn($i) print_stable"
}

# Print the routing table for each node
for {set i 0} {$i < $sz} {incr i} {
	$ns at 4.9 "$nn($i) print_rtable"
}

# Bring down a link
$ns rtmodel-at 5.0 down $n(1) $n(8) 
$ns at 5.0 "$nn(1) link_break 8"
$ns at 5.1 "$nn(8) link_break 1"

# Print the routing table on link down for each node
for {set i 0} {$i < $sz} {incr i} {
	$ns at 8.9 "$nn($i) print_dtable"
}

# Repair a link
$ns rtmodel-at 9.0 up $n(1) $n(8) 
$ns at 9.0 "$nn(1) link_up 8"
$ns at 9.1 "$nn(8) link_up 1"

# Print the routing table on link up for each node
for {set i 0} {$i < $sz} {incr i} {
	$ns at 10.8 "$nn($i) print_utable"
}

# Set stop time for AntNet algorithm
for {set i 0} {$i < $sz} {incr i} {
	$ns  at 11.0 "$nn($i) stop"
}

# Final Wrap up
proc Finish {} {
	global ns tf nf
	$ns flush-trace
    	close $nf
        close $tf
}

$ns  at 12.0 "Finish"

# Start the simulator
$ns  run
