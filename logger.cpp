#include "logger.h"
#include "locks.h"

#define LOG_FILENAME ("log.txt")

pthread_mutex_t log_lock;
ofstream log_file;

bool init_logger()
{
    pthread_mutex_init(&log_lock, nullptr);
    log_file = ofstream(LOG_FILENAME);
    return log_file.is_open();
}

void log(string message)
{
    pthread_mutex_lock(&log_lock);
    log_file << message << endl;
    cout << message << endl; // TODO: Remove me
    pthread_mutex_unlock(&log_lock);
}

void destroy_logger()
{
    log_file.close();
}
