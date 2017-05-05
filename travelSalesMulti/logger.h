#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <pthread.h>

extern bool enableLog;

using namespace std;

class Logger
{
private:
    Logger()
    {
        pthread_mutex_init(&logMutex, NULL);
    }

    static Logger *loggerInstance;
    ofstream out;
public:
    pthread_mutex_t logMutex;
    static Logger* getInstance();
    void logStdout(string);
};

#endif // LOGGER_H
