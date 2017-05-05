#scp travel bharat@10.42.0.1:~
#mpiexec -np 3 --hostfile host_file.txt travel manavGraph.txt -v
mpiexec -np 6 travel mySampleGraph.txt -v
