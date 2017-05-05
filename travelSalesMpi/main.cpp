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
bool enableLog = false;
Logger *logger;

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
    if(argc == 2)
    {
        graph = new Graph(argv[1]);
        tree = new Tree(graph);
    }
    else if(argc == 3)
    {
        graph = new Graph(argv[1]);
        tree = new Tree(graph);
        if(strcmp(argv[2], "-v") == 0)
            enableLog = true;
    }
    else
    {
        cout<<"Exceeds or needs Parameter"<<endl;
        cout<<"Usage : mpiexec -np <no. of processes> --hostfile <hostFile> travel <graph file> -v"
              "Add -v in the end for verbose else not"
              "Make sure there is no space in the filename"<<endl;
        return 0;
    }

    /*
     * 1. Initialize the MPI
     * 2. Get the total number of processes running
     * 3. Get the rank of the current process
     */

    MPI_Init(&argc, &argv);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &tree->myId);
    logger = Logger::getInstance("log" + to_string(tree->myId) + ".txt");

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

    logger->logStdout("World Size = " + to_string(world_size) + "\n");
    if(tree->myId == 0)
    {
        logger->logStdout("Starting with the Main Process\n");
        graph->printGraphMatrix(graph->graphMatrix);
        logger->logStdout("\nCost of the Root Node: " + to_string(root->lowerBoundCost) + "\n");
        tree->priorityQueue.push_back(root);

        /*
         * 1. We keep on generating node till we have enough nodes to give to all
         *    the processes.
         * 2. We break at the time when number of nodes is equal to the number
         *    of slave processes
         */

        while(true)
        {
            Node *tempNode = tree->priorityQueue.front();
            tree->priorityQueue.pop_front();
            tree->branchNode(tempNode);
            if(tree->priorityQueue.size() == world_size - 1)
            {
                logger->logStdout("\nNumber of nodes added = " + to_string(tree->priorityQueue.size()) + "\n");
                break;
            }
        }

        //Send all the nodes to all the processes
        int processNum = 1;
        while (!tree->priorityQueue.empty())
        {
            vector<int> send_vec;
            for (int i = 1; i <= graph->numberOfVertices; i++)
            {
                for (int j = 1; j <= graph->numberOfVertices; j++)
                {
                    send_vec.push_back(tree->priorityQueue.front()->boolGraphMatrix[i][j]);
                }
            }
            tree->priorityQueue.pop_front();
            MPI_Send(send_vec.data(), send_vec.size(), MPI_INT, processNum, FIRST_NODE, MPI_COMM_WORLD);
            logger->logStdout("Sent to process id : " + to_string(processNum) + "\n");
            processNum++;
        }
        processNum = 1;
        int tempPathCost;
        int pathCost = 99999;
        string path;
        int flag;
        MPI_Status status;

        while(processNum < world_size)
        {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            /*
             * 1. We first probe here for intermediate cost and path, i.e. when a
             *    process get to the leaf node and generates a path and cost
             * 2. We then send this path and cost to all the other processes
             */

            if(status.MPI_TAG == INTERMEDIATE_COST)
            {
                int number_amount;
                MPI_Get_count(&status, MPI_CHAR, &number_amount);
                char *pathCostPath = (char*)malloc(sizeof(char) * number_amount);
                MPI_Status statusRecv;
                MPI_Recv(pathCostPath, number_amount, MPI_CHAR, MPI_ANY_SOURCE, INTERMEDIATE_COST, MPI_COMM_WORLD, &statusRecv);
                logger->logStdout("Received from : " + to_string(statusRecv.MPI_SOURCE)
                                  + ", " + pathCostPath + "\n");
                for(int i = 1; i<world_size;i++)
                {
                    if(statusRecv.MPI_SOURCE != i)
                    {
                        MPI_Send(pathCostPath, number_amount, MPI_CHAR, i, INTERMEDIATE_COST, MPI_COMM_WORLD);
                        logger->logStdout("Sent to process : " + to_string(i) + "\n");
                    }
                }
            }

            /*
             * 1. If the message sent by all the process is not the intermediate
             *    path and cost but the final path and cost i.e. when they are
             *    ending, then we receive it and update a path and cost min of all
             */

            else if(status.MPI_TAG == FINAL_COST)
            {
                int number_amount;
                MPI_Get_count(&status, MPI_CHAR, &number_amount);
                char *pathCostPath = (char*)malloc(sizeof(char) * number_amount);
                MPI_Recv(pathCostPath, number_amount, MPI_CHAR, MPI_ANY_SOURCE,
                         FINAL_COST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                stringstream ss;
                ss<<pathCostPath;
                logger->logStdout("\nFinal Received from : " + to_string(processNum) + ", " + ss.str());
                int i=0;
                string str[2];
                string token;
                while(getline(ss, token, ','))
                    str[i++] = token;
                tempPathCost = atoi(str[0].c_str());
                if(tempPathCost < pathCost)
                {
                    pathCost = tempPathCost;
                    path = str[1].c_str();
                }
                processNum++;
            }
        }
        logger->logStdout("\n\nFinal Path Cost = " + to_string(pathCost) + ", path = " + path + "\n");
    }
    else if(tree->myId > 0)
    {
        /*
         * 1. If the process is a child process then then it receives the node
         *    call branch node for further processing
         */
        logger->logStdout("Starting with process id : " + to_string(tree->myId) + "\n");
        int recv_data[graph->numberOfVertices * graph->numberOfVertices];
        MPI_Recv(recv_data, graph->numberOfVertices * graph->numberOfVertices, MPI_INT,
                 0, FIRST_NODE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        Node *tempNode = new Node(graph->numberOfVertices);
        logger->logStdout("Node received : \n");
        int count = 0;
        for (int i = 1; i <= graph->numberOfVertices; i++)
        {
            for (int j = 1; j <= graph->numberOfVertices; j++)
            {
                tempNode->boolGraphMatrix[i][j] = recv_data[count++];
                logger->logStdout("\t" + to_string(tempNode->boolGraphMatrix[i][j]));
            }
            logger->logStdout("\n");
        }
        tempNode->nodeNumber = 1;
        tempNode->lowerBoundCost = tree->lowerBoundCostCalc(tempNode);
        tree->priorityQueue.push_back(tempNode);
        tree->processGraph();
    }

    MPI_Finalize();

    auto finishTime = std::chrono::high_resolution_clock::now();
    cout<<"\n" << std::chrono::duration_cast<std::chrono::nanoseconds>(finishTime-startTime).count() << "ns\n";
    return 0;
}
