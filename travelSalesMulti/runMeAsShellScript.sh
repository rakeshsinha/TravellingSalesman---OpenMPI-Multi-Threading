g++ -c -m64 -pipe -O2 -std=c++0x -Wall -W -o main.o main.cpp
g++ -c -m64 -pipe -O2 -std=c++0x -Wall -W -o graph.o graph.cpp
g++ -c -m64 -pipe -O2 -std=c++0x -Wall -W -o tree.o tree.cpp
g++ -c -m64 -pipe -O2 -std=c++0x -Wall -W -o logger.o logger.cpp
g++ -m64 -pthread -Wl,-O1 -o travel main.o graph.o tree.o logger.o
