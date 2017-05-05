/*
 * Rakesh Sinha - rakesh.ero@gmail.com
 * Student Id - 014994744
 */

#include <fstream>
#include <algorithm>

#include "graph.h"

#define MAX 9999


/*
 * Constructor parses the file and fills the graphMatrix
 * 1,2 in the array will define edge from vertex 1 to 2
 * and the content at that position will be its weight
 */

Graph::Graph(char *fileName)
{
    string line, token;
    stringstream ss;

    ifstream in(fileName);
    if (in.is_open())
    {
        getline(in, line);
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        int numberOfVertices = atoi(line.c_str());
        this->numberOfVertices = numberOfVertices;

        graphMatrix = new int*[numberOfVertices + 1];
        for(int i = 1; i <= numberOfVertices; i++)
            graphMatrix[i] = new int[numberOfVertices + 1];

        for (int i = 1; i <= this->numberOfVertices; i++)
            for (int j = 1; j <= this->numberOfVertices; j++)
                graphMatrix[i][j] = MAX;

        while(getline(in, line))
        {
            ss.str("");
            ss.clear();
            ss.str(line);
            string str[3];
            int i=0;
            while(getline(ss, token, ','))
                str[i++] = token;

            graphMatrix[atoi(str[0].c_str())][atoi(str[1].c_str())] = atoi(str[2].c_str());
            graphMatrix[atoi(str[1].c_str())][atoi(str[0].c_str())] = atoi(str[2].c_str());
        }
    }
    in.close();
}


/*
 * Prints the original Graph Matrix
 */

void Graph::printGraphMatrix()
{
    cout<<"\n The original matrix :\n";
    for (int i = 1; i <= this->numberOfVertices; i++)
    {
        cout<<"\n";
        for (int j = 1; j <= this->numberOfVertices; j++)
            cout<<"\t"<<graphMatrix[i][j];
    }
}
