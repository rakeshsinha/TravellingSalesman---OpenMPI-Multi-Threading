#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>

extern bool enableLog;

using namespace std;

class Logger
{
private:
    Logger(string logFileName)
    {
        logFile = logFileName;
    }

    static Logger *loggerInstance;
    string logFile;
public:
    static Logger* getInstance(string);
    void logStdout(string);
};

#endif // LOGGER_H
