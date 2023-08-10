#ifndef _LOGGER_H
#define _LOGGER_H

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

bool init_logger();
void log(string message);
void destroy_logger();

#endif //_LOGGER_H
