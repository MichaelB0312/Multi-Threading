#ifndef _BANK_MANAGER_H
#define _BANK_MANAGER_H

#include "locks.h"

using namespace std;

typedef struct account {
    int account_num;
    int password;
    int balance;
    struct readers_writers account_lock;
}account;

typedef struct BANK {
    list<account> accounts_list;
    struct readers_writers accounts_lock;

    int bank_balance;
    struct readers_writers bank_balance_lock;

    pthread_t commission_worker_thread;
    
    bool should_close;
    pthread_mutex_t should_close_lock;
}BANK;



void init_bank(BANK* bank);
void close_bank(BANK* bank);

// The caller to this function must have the accounts_lock locked for read or write
bool does_account_exist(BANK* bank, int acc_num);

#endif // _BANK_MANAGER_H
