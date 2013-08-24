############### Generations

#total fwd
echo "Total forward ants generated is ";
grep 'One forward ant created' antnet_op.log | wc -l;

#total bk
echo "Total backward ants generated is ";
grep 'One backward ant created' antnet_op.log | wc -l;

#gen at 1
echo "Packets generated at node 1 is ";
grep 'sending ant packet from 1' antnet_op.log | wc -l;

#gen at 8
echo "Packets generated at node 8 is ";
grep 'sending ant packet from 8' antnet_op.log | wc -l;



############### Losses

echo "-----------------------------------";

# transmission loss
echo "Packets lost during transmission is ";
grep lost antnet_op.log | wc -l;

#auto fwd loss
echo "Forward ants lost due to path errors is ";
grep 'One forward ant auto' antnet_op.log | wc -l;

#auto bk loss
echo "Backward ants lost due to path errors is ";
grep 'Two forward ants auto' antnet_op.log | wc -l;



############### Skipped

echo "-----------------------------------";

#skipped
echo "Forward ants generation skip count is ";
grep 'skipped' antnet_op.log | wc -l;



############### Through

echo "-----------------------------------";

#total through 1
echo "Number of passes through node 1 is ";
grep 'Adding node 1' antnet_op.log | wc -l;

#total through 8
echo "Number of passes through node 8 is";
grep 'Adding node 8' antnet_op.log | wc -l;

#total through 2
echo "Number of passes through node 2 is";
grep 'Adding node 2' antnet_op.log | wc -l



############### Next hop 1

echo "-----------------------------------";

#fa gen at 8, next hop 1
echo "Forward ants generated at node 8, with next hop 1 is ";
grep 'One forward ant created' antnet_op.log | grep 'sending ant packet from 8' | grep 'with next hop 1' | wc -l;

#ba gen at 8, next hop 1
echo "Backward ants generated at node 8, with next hop 1 is ";
grep 'One backward ant created' antnet_op.log | grep 'sending ant packet from 8' | grep 'with next hop 1' | wc -l;

#fa through 8, next hop 1
echo "Forward ants transmitted from node 8, with next hop 1 is ";
grep 'Fowarding forward ant from 8 to 1' antnet_op.log | wc -l;

#ba through 8, next hop 1
echo "Backward ants transmitted from node 8, with next hop 1 is ";
grep 'Fowarding backward ant from 8 to 1' antnet_op.log | wc -l;

#gen at 0/10/11, next hop 1
echo "Transmissions from nodes 0, 10 and 11 with next hop 1 is ";
grep 'sending ant packet' antnet_op.log | grep -e 'from 0' -e 'from 10' -e 'from 11' |grep 'with next hop 1' | wc -l;

#total next hop 1
echo "Total transmissions with next hop 1 is ";
grep 'sending ant packet from' antnet_op.log | grep 'with next hop 1' | wc -l;



############### Next hop 8

echo "-----------------------------------";

#fa gen at 1, next hop 8
echo "Forward ants generated at node 1, with next hop 8 is ";
grep 'ant created' antnet_op.log | grep 'sending ant packet from 1' | grep 'with next hop 8' | wc -l;

#ba gen at 1, next hop 8
echo "Backward ants generated at node 1, with next hop 8 is ";
grep 'One backward ant created' antnet_op.log | grep 'sending ant packet from 1' | grep 'with next hop 8' | wc -l;

#fa through 1, next hop 8
echo "Forward ants transmitted from node 1, with next hop 8 is ";
grep 'Fowarding forward ant from 1 to 8' antnet_op.log | wc -l;

#ba through 1, next hop 8
echo "Backward ants transmitted from node 1, with next hop 8 is ";
grep 'Fowarding backward ant from 1 to 8' antnet_op.log | wc -l;

#total next hop 8
echo "Total transmissions with next hop 8 is ";
grep 'sending ant packet from' antnet_op.log | grep 'with next hop 8' | wc -l;



############### Between 1 and 2

echo "-----------------------------------";

#fa through 2, next hop 1
echo "Forward ants transmitted from node 2, with next hop 1 is ";
grep 'Fowarding forward ant from 2 to 1' antnet_op.log | wc -l;

#ba through 2, next hop 1
echo "Backward ants transmitted from node 2, with next hop 1 is ";
grep 'Fowarding backward ant from 2 to 1' antnet_op.log | wc -l;

#fa through 1, next hop 2
echo "Forward ants transmitted from node 1, with next hop 2 is ";
grep 'Fowarding forward ant from 1 to 2' antnet_op.log | wc -l;

#ba through 1, next hop 2
echo "Backward ants transmitted from node 1, with next hop 2 is ";
grep 'Fowarding backward ant from 1 to 2' antnet_op.log | wc -l;

############### Drops

echo "-----------------------------------";

#total drop
echo "Total packets dropped is ";
grep ^d antnet_trace.out | wc -l;

#drop at link break, from 8 to 1
echo "Packets from node 8 to 1 dropped due to link break is ";
grep 'd 5 8 1' antnet_trace.out | wc -l;

#drop at link break, from 1 to 8
echo "Packets from node 1 to 8 dropped due to link break is ";
grep 'd 5 1 8' antnet_trace.out | wc -l;
