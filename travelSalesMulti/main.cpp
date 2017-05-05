/*
 * Rakesh Sinha - rakesh.ero@gmail.com
 * Student Id - 014994744
 */

#include <cstring>
#include <chrono>

#include "graph.h"
#include "tree.h"
#include "logger.h"

using namespace std;
int threadCount;
bool enableLog = false;


/*
 * 1. Takes input of the graph from the file
 * 2. Graph Object is created over the graph file, Graph Constructor
 *    parse that file into a Square Matrix with weight as its content
 * 3. Tree Object takes Graph Object as input to process over the graph
 */

int main(int argc, char *argv[])
{
    auto startTime = std::chrono::high_resolution_clock::now();
    Graph *graph;
    Tree *tree;
    if(argc == 3)
    {
        graph = new Graph(argv[1]);
        tree = new Tree(graph);
        if(atoi(argv[2]) <= 1)
        {
            cout<<"\nYou need atleast two threads to continue with";
            return 0;
        }
        threadCount = atoi(argv[2]);
    }
    else if(argc == 4)
    {
        graph = new Graph(argv[1]);
        tree = new Tree(graph);
        if(atoi(argv[2]) <= 1)
        {
            cout<<"\nYou need atleast two threads to continue with";
            return 0;
        }
        threadCount = atoi(argv[2]);
        if(strcmp(argv[3], "-v") == 0)
            enableLog = true;
    }
    else
    {
        cout<<"Exceeds or needs Parameter"<<endl;
        cout<<"Usage : ./travel <filename> <no. of threads>"
              "Add -v in the end for verbose else not"
              "Make sure there is no space in the filename"<<endl;
        return 0;
    }

    graph->printGraphMatrix();

    /*
     * 1. Root node is created and pushed in to the priority queue
     * 2. "boolGraphMatrix" is the Matrix that is used to keep track
     *    of the visited and not visited nodes
     * 3. Initially all the edges are marked as 1 (since its root and none
     *    of the edges are visited, if visited then 0) and same edges as 2
     *    i.e. 1,1 is the same vetex so in Matrix 1,1 is marked as 2
     */

    Node *root = new Node(graph->numberOfVertices);
    for (int i = 1; i <= graph->numberOfVertices; i++)
    {
        for (int j = 1; j <= graph->numberOfVertices; j++)
            if (i == j)
                root->boolGraphMatrix[i][j] = 2;
            else
                root->boolGraphMatrix[i][j] = 1;
    }

    root->lowerBoundCost = tree->lowerBoundCostCalc(root)/2;
    root->nodeNumber = 0;
    cout<<"\nCost of the Root Node: "<<root->lowerBoundCost<<"\n\n";

    tree->priorityQueue.push_back(root);

    /*
     * This function starts all the Threads
     */

    tree->start(threadCount);

    auto finishTime = std::chrono::high_resolution_clock::now();
    cout << std::chrono::duration_cast<std::chrono::nanoseconds>(finishTime-startTime).count() << "ns\n";
    return 0;
}
