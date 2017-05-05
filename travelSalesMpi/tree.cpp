/*
 * Rakesh Sinha - rakesh.ero@gmail.com
 * Student Id - 014994744
 */

#include <cstring>

#include "tree.h"

int pathCost = 99999;
int checkWaiting = 0;


/*
 * Initially every node will have a Matrix initialzed for the
 * Excluded and Included nodes
 */

Node::Node(int numberOfVertices)
{
    boolGraphMatrix = new int*[numberOfVertices + 1];
    for(int i = 1; i <= numberOfVertices; i++)
        boolGraphMatrix[i] = new int[numberOfVertices + 1];
}


/*
 * This will have the Graph
 */

Tree::Tree(Graph *graph)
{
    this->graph = graph;
}


/*
 * 1. Check if we have a message of path and cost
 * 2. If it is then update the path and cost and pop all from queue that are
 *    greater than this.
 */

void Tree::processGraph()
{
    logger->logStdout("Processing the Node : " + to_string(myId) + "\n");
    Node *temp = NULL;
    while (!priorityQueue.empty())
    {
        string strArray[2];
        int tempPathCost = 999999;
        int flag = 9;
        MPI_Status status;
        MPI_Iprobe(0, INTERMEDIATE_COST, MPI_COMM_WORLD, &flag, &status);
        if(flag == 1)
        {
            int number_amount;
            MPI_Get_count(&status, MPI_CHAR, &number_amount);
            char *pathCostPath = (char*)malloc(sizeof(char) * number_amount);
            MPI_Recv(pathCostPath, number_amount, MPI_CHAR, 0,
                     INTERMEDIATE_COST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            stringstream ss;
            ss<<pathCostPath;
            logger->logStdout("Received from root : " + ss.str() + "\n");
            int i=0;
            string token;
            while(getline(ss, token, ','))
                strArray[i++] = token;
            tempPathCost = atoi(strArray[0].c_str());
        }
        if(tempPathCost < pathCost)
        {
            logger->logStdout("Message : Changing pathCost, from : " + to_string(pathCost) + " to : " + to_string(tempPathCost) + "\n");
            pathCost = tempPathCost;
            path = strArray[1].c_str();
            list<Node*>::iterator it = priorityQueue.begin();
            while(it != priorityQueue.end())
            {
                if(pathCost<=(*it)->lowerBoundCost)
                {
                    logger->logStdout("Popping node of cost = " + to_string((*it)->lowerBoundCost) + "\n");
                    it = priorityQueue.erase(it);
                }
                else
                    it++;
            }
        }
        if(!priorityQueue.empty())
        {
            temp = priorityQueue.front();
            priorityQueue.pop_front();
            branchNode(temp);
        }
    }
    /*
     * 1. If all queues are empty then send final path and cost to root
     */
    logger->logStdout("\nSending final cost and path to Root = " + to_string(pathCost));
    stringstream ss;
    ss<<pathCost<<","<<path;
    string tempStr = ss.str();
    MPI_Send(tempStr.c_str(), tempStr.length(), MPI_CHAR, 0, FINAL_COST, MPI_COMM_WORLD);
    logger->logStdout("\nEnding with processing Node : " + to_string(myId) + "\n");
}


/*
 * 1. Layout two branches from a node
 * 2. The check is made is such that there will always be two branches out of one Node
 * 3. Initially root Node's Matrix value was assigned 1, so now when we include an edge
 *    we mark that block as 0 and if we exclude a node then we mark that block as -1
 * 4. When branching we find that if there are two edges fixed at a vertex then we move
 *    on to the next vertex and till the first occurance of 1 and branch at that position
 */

void Tree::branchNode(Node *node)
{
    if (node == NULL)
        return;
    Node *n1 = NULL, *n2 = NULL;
    bool comeOut = false;
    for (int i = 1; i <= graph->numberOfVertices && comeOut == false; i++)
    {
        for (int j = 1; j <= graph->numberOfVertices && comeOut == false; j++)
        {
            int checkForZero = 0, checkForOne = 0;
            for (int k = 1; k <= graph->numberOfVertices && comeOut == false; k++)
            {
                if(node->boolGraphMatrix[i][k] == 0)
                    checkForZero++;
                if(node->boolGraphMatrix[i][k] == 1)
                    checkForOne++;
            }
            if(checkForZero == 2)
            {
                logger->logStdout("Branching not possible at level : " + to_string(i) + "\n");
                break;
            }

            if(node->boolGraphMatrix[i][j] == 1)
            {
                logger->logStdout(" Branching at [" + to_string(i) + "]" + "[" + to_string(j) +
                        "], value = " + to_string(node->boolGraphMatrix[i][j]) + "\n");
                n1 = new Node(graph->numberOfVertices);
                n2 = new Node(graph->numberOfVertices);

                for (int i = 1; i <= graph->numberOfVertices; i++)
                    for (int j = 1; j <= graph->numberOfVertices; j++)
                    {
                        n1->boolGraphMatrix[i][j] = node->boolGraphMatrix[i][j];
                        n2->boolGraphMatrix[i][j] = node->boolGraphMatrix[i][j];
                    }

                //Include edges are marked 0, exclude as 1
                n1->boolGraphMatrix[i][j] = 0;
                n1->boolGraphMatrix[j][i] = 0;
                n2->boolGraphMatrix[i][j] = -1;
                n2->boolGraphMatrix[j][i] = -1;
                n1->nodeNumber = node->nodeNumber + 1;
                n2->nodeNumber = n1->nodeNumber + 1;


                /*
                 * 1. Rearrange the Matrix now.
                 * 2. If for any row there are two 0 then that edge has two fixed edges
                 *    so exclude all other edges, hence mark all other as -1
                 * 3. If there are two 1 then and rest as -1 then we need to include
                 *    those two edges so mark them both as 0
                 * 4. If there is one 0 and one 1 then include that edges and mark it as 0
                 */
                for (int i = 1; i <= graph->numberOfVertices; i++)
                {
                    int checkForMinusOne = 0;
                    checkForZero = 0; checkForOne = 0;

                    for (int j = 1; j <= graph->numberOfVertices; j++)
                    {
                        if (n1->boolGraphMatrix[i][j] == 0)
                            checkForZero++;
                        if (n1->boolGraphMatrix[i][j] == 1)
                            checkForOne++;
                        if (n1->boolGraphMatrix[i][j] == -1)
                            checkForMinusOne++;
                    }
                    if (checkForZero == 2)
                    {
                        for (int j = 1; j <= graph->numberOfVertices; j++)
                            if (n1->boolGraphMatrix[i][j] == 1)
                            {
                                n1->boolGraphMatrix[i][j] = -1;
                                n1->boolGraphMatrix[j][i] = -1;
                            }
                    }
                    if (checkForZero ==1 && checkForOne == 1)
                    {
                        for (int j = 1; j <= graph->numberOfVertices; j++)
                            if (n1->boolGraphMatrix[i][j] == 1)
                            {
                                n1->boolGraphMatrix[i][j] = 0;
                                n1->boolGraphMatrix[j][i] = 0;
                            }
                    }
                    if (graph->numberOfVertices - checkForMinusOne - 1 == 2 &&
                            checkForOne == 2)
                    {
                        for (int j = 1; j <= graph->numberOfVertices; j++)
                            if (n1->boolGraphMatrix[i][j] == 1)
                            {
                                n1->boolGraphMatrix[i][j] = 0;
                                n1->boolGraphMatrix[j][i] = 0;
                            }
                    }


                    checkForZero = 0; checkForOne = 0; checkForMinusOne = 0;
                    for (int j = 1; j <= graph->numberOfVertices; j++)
                    {
                        if (n2->boolGraphMatrix[i][j] == 0)
                            checkForZero++;
                        if (n2->boolGraphMatrix[i][j] == 1)
                            checkForOne++;
                        if (n2->boolGraphMatrix[i][j] == -1)
                            checkForMinusOne++;
                    }
                    if(checkForZero == 2)
                    {
                        for (int j = 1; j <= graph->numberOfVertices; j++)
                            if (n2->boolGraphMatrix[i][j] == 1)
                            {
                                n2->boolGraphMatrix[i][j] = -1;
                                n2->boolGraphMatrix[j][i] = -1;
                            }
                    }

                    if(checkForZero ==1 && checkForOne == 1)
                    {
                        for (int j = 1; j <= graph->numberOfVertices; j++)
                            if (n2->boolGraphMatrix[i][j] == 1)
                            {
                                n2->boolGraphMatrix[i][j] = 0;
                                n2->boolGraphMatrix[j][i] = 0;
                            }
                    }
                    if (graph->numberOfVertices - checkForMinusOne - 1 == 2 &&
                            checkForOne == 2)
                    {
                        for (int j = 1; j <= graph->numberOfVertices; j++)
                            if (n2->boolGraphMatrix[i][j] == 1)
                            {
                                n2->boolGraphMatrix[i][j] = 0;
                                n2->boolGraphMatrix[j][i] = 0;
                            }
                    }
                }

                for (int i = 1; i <= graph->numberOfVertices; i++)
                {
                    logger->logStdout("\n");
                    for (int j = 1; j <= graph->numberOfVertices; j++)
                        logger->logStdout("\t" + to_string(n1->boolGraphMatrix[i][j]));
                }
                logger->logStdout("\n");
                for (int i = 1; i <= graph->numberOfVertices; i++)
                {
                    logger->logStdout("\n");
                    for (int j = 1; j <= graph->numberOfVertices; j++)
                        logger->logStdout("\t" + to_string(n2->boolGraphMatrix[i][j]));
                }
                logger->logStdout("\n");

                n1->lowerBoundCost = lowerBoundCostCalc(n1)/2;
                logger->logStdout("Cost of [" + to_string(i) + "]" + "[" + to_string(j) + "] : " +
                        to_string(n1->boolGraphMatrix[i][j]) + " --> " + to_string(n1->lowerBoundCost) + "\n");

                n2->lowerBoundCost = lowerBoundCostCalc(n2)/2;
                logger->logStdout("Cost of [" + to_string(i) + "]" + "[" + to_string(j) + "] : " +
                        to_string(n2->boolGraphMatrix[i][j]) + " --> " + to_string(n2->lowerBoundCost) + "\n");
                comeOut = true;
            }
        }
    }

    if(myId == 0)
    {
        priorityQueue.push_back(n1);
        priorityQueue.push_back(n2);
        return;
    }

    /*
     * 1. Check for 0's on every row, so we are basically checking if each row has two 0's
     * 2. If all row has two 0 it means its the last Node or the leaf Node
     * 3. Now compare both the nodes and whoever has less weight find path for it.
     */

    int checkForZeroN1 = 0, allVerticesChekedN1 = 0;
    int checkForZeroN2 = 0, allVerticesChekedN2 = 0;

    for (int i = 1; i <= graph->numberOfVertices && n1 != NULL && n2 != NULL; i++)
    {
        checkForZeroN1 = 0; checkForZeroN2 = 0;
        for (int j = 1; j <= graph->numberOfVertices; j++)
        {
            if(n1->boolGraphMatrix[i][j] == 0)
                checkForZeroN1++;
            if(n2->boolGraphMatrix[i][j] == 0)
                checkForZeroN2++;
        }
        if(checkForZeroN1 == 2)
            allVerticesChekedN1++;
        if(checkForZeroN2 == 2)
            allVerticesChekedN2++;
    }

    if(allVerticesChekedN1 == graph->numberOfVertices)
    {
        logger->logStdout("N1 is a leaf node\n");
        list<Node*>::iterator it = priorityQueue.begin();
        while(it != priorityQueue.end())
        {
            if(n1->lowerBoundCost<=(*it)->lowerBoundCost)
            {
                logger->logStdout("Popping node of cost = " + to_string((*it)->lowerBoundCost) + "\n");
                it = priorityQueue.erase(it);
            }
            else
                it++;
        }

        if(n1->lowerBoundCost<=n2->lowerBoundCost)
        {
            logger->logStdout("N1 is a potential Final Matrix\n");
            if(n1->lowerBoundCost < pathCost)
            {
                path = findPath(n1);
                logger->logStdout("Sending this information to root\n");
                stringstream ss;
                ss<<pathCost<<","<<path;
                string str = ss.str();
                //If we have a cost then send to root to send to all
                MPI_Send(str.c_str(), str.length(), MPI_CHAR, 0, INTERMEDIATE_COST, MPI_COMM_WORLD);
            }
            else
                logger->logStdout("N1 < global path cost, so no path caculation");
            return;
        }
    }
    if(allVerticesChekedN2 == graph->numberOfVertices)
    {
        logger->logStdout("N2 is a leaf node\n");
        list<Node*>::iterator it = priorityQueue.begin();
        while(it != priorityQueue.end())
        {
            if(n2->lowerBoundCost<=(*it)->lowerBoundCost)
            {
                logger->logStdout("Popping node of cost = " + to_string((*it)->lowerBoundCost) + "\n");
                it = priorityQueue.erase(it);
            }
            else
                it++;
        }

        if(n2->lowerBoundCost<=n1->lowerBoundCost)
        {
            logger->logStdout("N2 is a potential Final Matrix\n");
            if(n2->lowerBoundCost < pathCost)
            {
                path = findPath(n2);
                logger->logStdout("Sending this information to root\n");
                stringstream ss;
                ss<<pathCost<<","<<path;
                string str = ss.str();
                //If we have a cost then send to root to send to all
                MPI_Send(str.c_str(), str.length(), MPI_CHAR, 0, INTERMEDIATE_COST, MPI_COMM_WORLD);
            }
            else
                logger->logStdout("N2 < global path cost, so no path caculation");
            return;
        }
    }
    if(n1 != NULL && n2 != NULL)
    {
        if(n1->lowerBoundCost<pathCost)
        {
            logger->logStdout("Pushing N1\n");
            priorityQueue.push_back(n1);
        }
        else
        {
            logger->logStdout("\nNot Pushing N1, since we already have lower bound of -- " + to_string(pathCost));
        }
        if(n2->lowerBoundCost<pathCost)
        {
            logger->logStdout("Pushing N2\n");
            priorityQueue.push_back(n2);
        }
        else
        {
            logger->logStdout("\nNot Pushing N2, since we already have lower bound of -- " + to_string(pathCost));
        }
        priorityQueue.sort(Tree::compareNodes);
        return;
    }
    logger->logStdout("Nothing Done in this Iteration\n");
}


/*
 * When calculating the cost look for the following
 * 1. Do we have two included edges i.e. two 0 in a row
 *      if yes then for that vertex they are the two edges
 * 2. Do we have one included edge
 *      Then take that edge and min of the other
 * 3. Do we have no included edge
 *      then we just find two minimums
 */

float Tree::lowerBoundCostCalc(Node *node)
{
    int tempCount = 0, fixTemp = 0;
    int min1, min2;
    int fix1, fix2;
    float lowerBoundCost = 0;
    int checkForZero = 0, checkForOne = 0;

    for (int i = 1; i <= graph->numberOfVertices; i++)
    {
        checkForZero = 0; checkForOne = 0;
        for (int j = 1; j <= graph->numberOfVertices; j++)
        {
            if(node->boolGraphMatrix[i][j] == 0)
                checkForZero++;
            else if(node->boolGraphMatrix[i][j] == 1)
                checkForOne++;
        }
        if(checkForZero + checkForOne < 2)
        {
            logger->logStdout("Invalid branching, setting cost to maximum\n");
            return 99999;
        }
    }

    for (int i = 1; i <= graph->numberOfVertices; i++)
    {
        min1 = 0; min2 = 0; tempCount = 0;
        for (int j = 1; j <= graph->numberOfVertices; j++)
        {
            if (node->boolGraphMatrix[i][j] == 1)
            {
                if (tempCount == 0)
                {
                    min1 = graph->graphMatrix[i][j];
                    tempCount++;
                }
                else if (tempCount == 1)
                {
                    if(min1>= graph->graphMatrix[i][j])
                    {
                        min2 = min1;
                        min1 = graph->graphMatrix[i][j];
                        tempCount++;
                    }
                    else
                    {
                        min2 = graph->graphMatrix[i][j];
                        tempCount++;
                    }
                }
                else
                {
                    if(min1 >= graph->graphMatrix[i][j])
                    {
                        if(min2 >= min1)
                            min2 = min1;
                        min1 = graph->graphMatrix[i][j];
                    }
                    if(graph->graphMatrix[i][j] > min1 && graph->graphMatrix[i][j] <=min2)
                        min2 = graph->graphMatrix[i][j];
                }
            }
        }

        fix1 = 0; fix2 = 0; fixTemp = 0;
        for (int j = 1; j <= graph->numberOfVertices; j++)
        {
            if (node->boolGraphMatrix[i][j] == 0 and fixTemp == 0)
            {
                fix1 = graph->graphMatrix[i][j];
                fixTemp++;
            }
            else if (node->boolGraphMatrix[i][j] == 0 and fixTemp == 1)
            {
                fix2 = graph->graphMatrix[i][j];
                fixTemp++;
            }
        }
        if(fixTemp == 0)
        {
            lowerBoundCost = lowerBoundCost + min1 + min2;
        }
        else if(fixTemp == 1)
        {
            lowerBoundCost = lowerBoundCost + min1 + fix1;
        }
        else if(fixTemp == 2)
        {
            lowerBoundCost = lowerBoundCost + fix1 + fix2;
        }
    }
    return lowerBoundCost;
}


/*
 * 1. Check if the received node's lowerBoundCost is less than the last calculated cost
 *    If yes then calculate path else return the previous path
 */

string Tree::findPath(Node *temp)
{
    if(pathCost > temp->lowerBoundCost)
    {
        pathCost = temp->lowerBoundCost;
        logger->logStdout("Global Path Cost = " + to_string(pathCost) + "\n");
        int checkBack = 1, current = 1;
        stringstream path;
        path<<current<<" ";
        while (true)
        {
            for (int j = 1; j <= graph->numberOfVertices; j++)
            {
                if(temp->boolGraphMatrix[current][j] == 0 && checkBack != j)
                {
                    path<<j<<" ";
                    checkBack = current;
                    current=j;
                    if(j==1)
                    {
                        logger->logStdout("Path = " + path.str() + "\n");
                        return path.str();
                    }
                    break;
                }
            }
        }
    }
    return this->path;
}
