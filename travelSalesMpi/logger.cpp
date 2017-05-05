#include "logger.h"

Logger* Logger::loggerInstance = NULL;

Logger* Logger::getInstance(string logFileName)
{
    if(!loggerInstance)
        loggerInstance = new Logger(logFileName);
    return loggerInstance;
}

/*
 * Opens a file logs into it and close it
 */
void Logger::logStdout(string str)
{
    if(enableLog)
    {
        ofstream myfile;
        myfile.open (logFile, std::ofstream::out | std::ofstream::app);
        myfile << str;
        myfile.close();
    }
}
