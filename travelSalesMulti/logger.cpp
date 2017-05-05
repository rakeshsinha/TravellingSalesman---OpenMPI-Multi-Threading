#include "logger.h"

Logger* Logger::loggerInstance = NULL;

Logger* Logger::getInstance()
{
    if(!loggerInstance)
        loggerInstance = new Logger();
    return loggerInstance;
}

void Logger::logStdout(string str)
{
    if(enableLog)
    {
        pthread_mutex_lock(&logMutex);
        cout<<str;
        pthread_mutex_unlock(&logMutex);
    }
}
