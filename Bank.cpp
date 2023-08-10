//**************************************************************************************
// @file Bank.cpp
// @brief minimal implementation of a bank, implemented with threads.
//
// TODO: Add information about program
//**************************************************************************************
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

#include "atm.h"
#include "locks.h"
#include "logger.h"
#include "bank_manager.h"

using namespace std;

//**************************************************************************************
// Definitions
//**************************************************************************************

//**************************************************************************************
// Globals
//**************************************************************************************



//**************************************************************************************
// Internal functions
//**************************************************************************************


//**************************************************************************************
// @name main
// @brief TODO: Add
// 
// @param argc TODO: Add
// @param argv TODO: Add
//**************************************************************************************
int main(int argc, char* argv[]) {

    if (argc == 1) cerr << "Bank error: illegal arguments" << endl;

    BANK global_bank;
    ATM* ATM_list = new ATM[argc-1];
    
    // Initialize stuff
    if (!init_logger()) {
        perror("Bank error : failed to open log file");
        return EXIT_FAILURE;
    }
    init_bank(&global_bank);

    // Initialize all ATMs before starting their threads
    for (int i = 1; i < argc; ++i) {
        if (!ATM_list[i-1].initialize_atm(i-1, argv[i], &global_bank)) {
            cerr << "Bank error: illegal arguments" << endl;
            exit(EXIT_FAILURE);
        }
    }

    //Launch the ATMs:
    for (int i = 1; i < argc; ++i) {
        int result = pthread_create(&(ATM_list[i-1].worker_thread), nullptr, &create_atm_worker, &(ATM_list[i-1]));
        if (result != 0) {
            perror("Bank error : pthread_create failed");
            return EXIT_FAILURE;
        }
    }

    // Wait for all ATMs threads to finish
    for (int i = 1; i < argc; ++i) {
        pthread_join(ATM_list[i-1].worker_thread, nullptr);
        ATM_list[i-1].destroy_atm();
    }

    close_bank(&global_bank);

    delete[] ATM_list;
    return EXIT_SUCCESS;
}
