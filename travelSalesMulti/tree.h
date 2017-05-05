/*
 * Rakesh Sinha - rakesh.ero@gmail.com
 * Student Id - 014994744
 */

#ifndef TREE_H
#define TREE_H

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>

#include "graph.h"
#include "logger.h"

/*
 * 1. stopThread flag is to stop branching of the nodes
 * 2. exitAll is to exit all the threads
 * 3. Logger is to log the messages on to the screen
 */

extern bool stopThread;
extern bool exitAll;
extern Logger *logger;


/*
 * 1. Every Node has a lowerBoundCost to save the cost at each level
 *    and a boolGraphMatrix to mark included or excluded edges
 * 2. nodeNumber is an extra parameter to compare nodes as to decide
 *    which node should be before in the priority queue in case two
 *    Node as the same lowerBoundCost
 */

class Node
{
public:
    Node(){}
    Node(int);
    int nodeNumber;
    float lowerBoundCost;
    int **boolGraphMatrix;
};


/*
 * 1. graph holds the original Graph constructed from the file
 * 2. priorityQueue holds up the Nodes with min. lowerBoundCost node
 *    at the top. priorityQueue is sort by the function compareNodes
 * 3. t is the vector of threads since at the runtime it is decided
 *    that how many threads will be created to a vector is used
 * 4. path holds the final path of the matrix
 * 5. Process the Graph based on the Matrix contained inside the Graph
 * 6. All the threads are created in the start()
 * 7. Branching is done in the processGraph()
 * 8. lowerBoundCostCalc calculates the lower bound of a node that is branched
 * 9. branchNode creates two Nodes out a given node with one including
 *    an edge and other excluding that edge
 * 10. findPath() -- finds the path of a Node
 */

class Tree
{
public:
    Tree(Graph*);
    Graph *graph;
    list<Node*> priorityQueue;
    vector<thread> t;
    mutex m1;
    mutex m2;
    condition_variable cv;
    string path;
    int numThreads;
    void start(int);
    void processGraph();
    float lowerBoundCostCalc(Node*);
    void branchNode(Node*);
    string findPath(Node*);
    static bool compareNodes(Node *a, Node *b)
    {
        if (a->lowerBoundCost==b->lowerBoundCost)
            return (a->nodeNumber>b->nodeNumber);
        return (a->lowerBoundCost<b->lowerBoundCost);;
    }
};

#endif // TREE_H
