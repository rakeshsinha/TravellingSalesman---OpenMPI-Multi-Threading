mpic++ -c -std=c++11 -o main.o main.cpp
mpic++ -c -std=c++11 -o graph.o graph.cpp
mpic++ -c -std=c++11 -o tree.o tree.cpp
mpic++ -c -std=c++11 -o logger.o logger.cpp
mpic++ -std=c++11 -o travel main.o graph.o tree.o logger.o
