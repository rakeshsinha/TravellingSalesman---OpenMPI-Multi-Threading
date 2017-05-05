/*
 * Rakesh Sinha - rakesh.ero@gmail.com
 * Student Id - 014994744
 */

#include <cstring>

#include "tree.h"

bool stopThread = true;
bool exitAll = false;
Logger *logger = Logger::getInstance();
int pathCost = 99999;
int checkWaiting = 0;

#define LOG(str) (logger->logStdout("\n" + ((ostringstream&)(ostringstream() << this_thread::get_id())).str() + str))

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
 * 1. This function has a never ending loop that keeps on poping a node and branching it
 * 2. First checks if priorityQueue is empty if its empty then it increments the counter
 *    checkWaiting.
 * 3. If all the threads are waiting then that is the condition to end thread execution.
 * 4. All thread waiting sets exitAll to true which means that no one can branch now and
 *    eventually all with come out of the loop
 */

void Tree::processGraph()
{
    string str;
    str = " Enter Thread";LOG(str);
    while (true)
    {
        Node *temp = NULL;
        str = " going for mutex1";LOG(str);
        unique_lock<mutex> lk(m1);
        str = " Accquired mutex1, queue size = " + to_string(priorityQueue.size());LOG(str);
        if(priorityQueue.empty())
        {
            m2.lock();
            checkWaiting++;
            if(checkWaiting == numThreads)
            {
                str = " All threads waiting";LOG(str);
                exitAll = true;
                cv.notify_one();
            }
            m2.unlock();
            str = " entering condition wait, checkWaiting = " + to_string(checkWaiting);LOG(str);
            cv.wait(lk);
            m2.lock();
            checkWaiting--;
            str = " leaving condition wait, checkWaiting = " + to_string(checkWaiting);LOG(str);
            if(!priorityQueue.empty())
            {
                temp = priorityQueue.front();
                priorityQueue.pop_front();
                str = " Picked Node Cost = " + to_string(temp->lowerBoundCost);LOG(str);
            }
            m2.unlock();
        }
        else if(!exitAll && !priorityQueue.empty())
        {
            temp = priorityQueue.front();
            priorityQueue.pop_front();
            str = " Picked Node Cost = " + to_string(temp->lowerBoundCost);LOG(str);
        }
        lk.unlock();
        str = " Released mutex1";LOG(str);
        if(!exitAll)
        {
            branchNode(temp);
        }
        else
        {
            cv.notify_one();
            break;
        }
    }
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
    string str;
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
                str = " Branching not possible at level : " + to_string(i);LOG(str);
                break;
            }

            if(node->boolGraphMatrix[i][j] == 1)
            {
                str = " Branching at [" + to_string(i) + "]" + "[" + to_string(j) +
                        "], value = " + to_string(node->boolGraphMatrix[i][j]);LOG(str);
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

                stringstream ss;
                for (int i = 1; i <= graph->numberOfVertices; i++)
                {
                    ss<<"\n";
                    for (int j = 1; j <= graph->numberOfVertices; j++)
                        ss<<"\t" + to_string(n1->boolGraphMatrix[i][j]);
                }
                str = ss.str();LOG(str);
                ss.str(""); ss.clear();
                ss<<"\n";
                for (int i = 1; i <= graph->numberOfVertices; i++)
                {
                    ss<<"\n";
                    for (int j = 1; j <= graph->numberOfVertices; j++)
                        ss<<"\t" + to_string(n2->boolGraphMatrix[i][j]);
                }
                str = ss.str();LOG(str);
                logger->logStdout("\n");

                n1->lowerBoundCost = lowerBoundCostCalc(n1)/2;
                str = " Cost of [" + to_string(i) + "]" + "[" + to_string(j) + "] : " +
                        to_string(n1->boolGraphMatrix[i][j]) + " --> " + to_string(n1->lowerBoundCost);LOG(str);

                n2->lowerBoundCost = lowerBoundCostCalc(n2)/2;
                str = " Cost of [" + to_string(i) + "]" + "[" + to_string(j) + "] : " +
                        to_string(n2->boolGraphMatrix[i][j]) + " --> " + to_string(n2->lowerBoundCost);LOG(str);
                comeOut = true;
            }
        }
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

    str = " wait for mutex 1,2";LOG(str);
    m1.lock();
    m2.lock();
    str = " acquired mutex 1,2";LOG(str);
    if(allVerticesChekedN1 == graph->numberOfVertices)
    {
        str = " N1 is a leaf node";LOG(str);
        list<Node*>::iterator it = priorityQueue.begin();
        while(it != priorityQueue.end())
        {
            if(n1->lowerBoundCost<=(*it)->lowerBoundCost)
            {
                str = " Popping node of cost = " + to_string((*it)->lowerBoundCost);LOG(str);
                it = priorityQueue.erase(it);
            }
            else
                it++;
        }

        if(n1->lowerBoundCost<=n2->lowerBoundCost)
        {
            str = " N1 is a potential Final Matrix";LOG(str);
            path = findPath(n1);
            str = " Path for N1 = " + path;LOG(str);
            m2.unlock();
            m1.unlock();
            str = " Released mutex2";LOG(str);
            return;
        }
    }
    if(allVerticesChekedN2 == graph->numberOfVertices)
    {
        str = " N2 is a leaf node";LOG(str);
        list<Node*>::iterator it = priorityQueue.begin();
        while(it != priorityQueue.end())
        {
            if(n2->lowerBoundCost<=(*it)->lowerBoundCost)
            {
                str = " Popping node of cost = " + to_string((*it)->lowerBoundCost);LOG(str);
                it = priorityQueue.erase(it);
            }
            else
                it++;
        }

        if(n2->lowerBoundCost<=n1->lowerBoundCost)
        {
            str = " N2 is a potential Final Matrix";LOG(str);
            path = findPath(n2);
            str = " Path for N2 = " + path;LOG(str);
            m2.unlock();
            m1.unlock();
            str = " Released mutex2";LOG(str);
            return;
        }
    }
    if(n1 != NULL && n2 != NULL)
    {
        if(n1->lowerBoundCost<pathCost)
        {
            str = " Pushing N1";LOG(str);
            priorityQueue.push_back(n1);
        }
        else
        {
            str = " Not Pushing N1, since we already have lower bound of -- " + to_string(pathCost);LOG(str);
        }
        if(n2->lowerBoundCost<pathCost)
        {
            str = " Pushing N2";LOG(str);
            priorityQueue.push_back(n2);
        }
        else
        {
            str = " Not Pushing N2, since we already have lower bound of -- " + to_string(pathCost);LOG(str);
        }
        priorityQueue.sort(Tree::compareNodes);
        m2.unlock();
        m1.unlock();
        str = " Released mutex 1,2";LOG(str);
        cv.notify_all();
        str = " Condition Signaled to All";LOG(str);
        return;
    }
    m2.unlock();
    m1.unlock();
    str = " Nothing Done Released mutex 1,2";LOG(str);
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
            logger->logStdout("\n" + ((ostringstream&)(ostringstream() << this_thread::get_id())).str() +
                    " Cannot calculate cost as this branching is invalid. Setting cost to maximum");
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
        cout<<"\n" + ((ostringstream&)(ostringstream() << this_thread::get_id())).str() <<" Starting to find the path";
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
                        cout<<"\n\n"<<"\n" + ((ostringstream&)(ostringstream() << this_thread::get_id())).str()<<" Path = "<<path.str()<<"\n";
                        return path.str();
                    }
                    break;
                }
            }
        }
    }
    return this->path;
}


/*
 * 1. This starts all the threads and wait for them to join
 * 2. All the threads are in a vector
 */
void Tree::start(int threadCount)
{
    numThreads = threadCount;
    for (int i = 0; i <threadCount; i++)
    {
        t.push_back(thread(&Tree::processGraph, this));
        logger->logStdout("\nCreated thread -- " + ((ostringstream&)(ostringstream() << t[i].get_id())).str());
    }

    auto joinT = t.begin();
    while (joinT != t.end())
    {
       joinT->join();
       logger->logStdout("\nJoined thread -- " + ((ostringstream&)(ostringstream() << joinT->get_id())).str());
       joinT++;
    }

    cout<<"\n All Threads done";
    cout<<"\n Final Path -- "<<path<<"\n";
    cout<<"\n Final Cost -- "<<pathCost<<"\n";
}
