#ifndef _ATM_H
#define _ATM_H

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

#include "bank_manager.h"

using namespace std;

typedef struct ATM {
    int id;
    pthread_t worker_thread;
    ifstream commands_file;
    BANK* bank;
    //int N; 
    
    //ATM methods:
    void* execute_command_file(void);

    void open_account(stringstream& line_ss);
    void deposit(stringstream& line_ss);
    void withdrawal(stringstream& line_ss);
    void get_balance(stringstream& line_ss);
    void quit_account(stringstream& line_ss);
    void transfer(stringstream& line_ss);

    bool initialize_atm(int id, char* commands_file_path, BANK* bank);
    void destroy_atm(void);
}ATM;

void * create_atm_worker(void* ATM_pointer);

#endif //_ATM_H
