clear;

echo "Entering antnet folder";
cd antnet;

echo "Entering lib sub-folder";
cd lib;

echo "Cleaning lib";
rm -rf *~ *.o;

echo "Touching header files";
touch *.h;

echo "Leaving lib sub-folder";
cd ..;

echo "Entering run sub-folder";
cd run;

echo "Cleaning run";
rm -rf *~ *.nam *.out *.txt *.log;

echo "Leaving run sub-folder";
cd ..;

echo "Leaving antnet folder";
cd ..;

echo "Cleaning old log files in current folder";
rm -rf make_op.log make_error.log;

echo "-------------------------------------------------------------";
echo "Compiling cc files - running make";
make > make_op.log 2> make_error.log;

if [ $? -gt 0 ] 
then
	echo "Make failed, please check the log make_error.log, exiting";
	exit 1;	
fi

rm -rf make_error.log;
echo "Make successful, please check the log make_op.log, continuing";

echo "Entering antnet folder";
cd antnet;

echo "Entering run sub-folder";
cd run;

echo "-------------------------------------------------------------";
echo "Executing the tcl file - running ns";
ns antnet.tcl > antnet_op.log 2> antnet_error.log;

if [ $? -gt 0 ]
then
	echo "Run failed, please check the log antnet/scripts/antnet_error.log";
	exit 2;
fi

rm -rf antnet_error.log;
echo "Run successfull, please check the log antnet/scripts/antnet_op.log and result files under antnet/scripts";


echo "-------------------------------------------------------------";
echo "Analyzing th output - running analyzer";
./analyzer.sh > antnet_analysis.txt 2> antnet_analysis_error.log;

if [ $? -gt 0 ]
then
	echo "Analysis failed, please check the log antnet/scripts/antnet_analysis_error.log";
	exit 3;
fi

rm -rf antnet_analysis_error.log;
echo "Analysis successfull, please check the analysis in antnet/scripts/antnet_analysis.txt";


echo "-------------------------------------------------------------";
echo "Leaving run sub-folder";
cd ..;

echo "Leaving antnet folder";
cd ..;
