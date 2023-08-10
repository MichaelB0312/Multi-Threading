#ifndef _LOCKS_H
#define _LOCKS_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <list>
#include <ctime>
#include <pthread.h>

using namespace std;

struct readers_writers {

    pthread_mutex_t rd_lock;
    pthread_mutex_t wrt_lock;
    int rd_count;

    void init();

    void enter_reader();
    void enter_writer();
    void leave_reader();
    void leave_writer();
};

#endif // _LOCKS_H
