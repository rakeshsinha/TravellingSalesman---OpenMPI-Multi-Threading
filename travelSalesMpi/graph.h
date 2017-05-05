/*
 * Rakesh Sinha - rakesh.ero@gmail.com
 * Student Id - 014994744
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <sstream>
#include <vector>
#include <array>

using namespace std;

/*
 * 1. Graph stores the total number of vertices to iterate for
 *    and a graphMatrix to save edges and weights
 * 2. We need to save number of vertices since its a dynamically
 *    allocated Matrix
 */

class Graph
{
public:
    Graph(char*);
    int numberOfVertices;
    int **graphMatrix;
    void printGraphMatrix(int**);
};

#endif // GRAPH_H
