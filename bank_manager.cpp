#include <algorithm>
#include "bank_manager.h"
#include "logger.h"

#define BANK_COMMISSION_SLEEP_TIME (3)
#define MAX_COMMISSION_PERCENT (5)

bool should_bank_worker_die(BANK* bank);
void * commission_worker(void* bank_pointer);


void init_bank(BANK* bank)
{
    // init bank struct
    bank->accounts_lock.init();
    bank->bank_balance = 0;
    bank->should_close = false;
    pthread_mutex_init(&(bank->should_close_lock), nullptr);
    bank->bank_balance_lock.init();
    

    // create commission worker
    pthread_create(&(bank->commission_worker_thread), nullptr, &commission_worker, bank);
    
    // seed rng engine with current time
    srand(time(nullptr));
}

bool does_account_exist(BANK* bank, int acc_num)
{
    auto it = find_if(bank->accounts_list.begin(), bank->accounts_list.end(), [acc_num](const account& acc) {
        return acc.account_num == acc_num;
        });
    return (it != bank->accounts_list.end());
}

void close_bank(BANK* bank)
{
    // signal to all bank threads that it is time to die
    pthread_mutex_lock(&bank->should_close_lock);
    bank->should_close = true;
    pthread_mutex_unlock(&bank->should_close_lock);

    // wait for all threads to die
    pthread_join(bank->commission_worker_thread, nullptr);
}

bool should_bank_worker_die(BANK* bank)
{
    bool should_die;
    pthread_mutex_lock(&bank->should_close_lock);
    should_die = bank->should_close;
    pthread_mutex_unlock(&bank->should_close_lock);
    return should_die;
}

void take_commissions(BANK* bank, int commission_percent)
{
    int total_commission_taken = 0;
    int current_commission;
    // lock account list for reading, for the whole duration of taking the commissions
    bank->accounts_lock.enter_reader();

    for (auto it = bank->accounts_list.begin(); it != bank->accounts_list.end(); ++it) {
        // lock account for writing
        it->account_lock.enter_writer();
        // take money from account
        current_commission = (it->balance * commission_percent) / 100;
        it->balance -= current_commission;
        total_commission_taken += current_commission;
        log("Bank: commissions of " + to_string(commission_percent) + " % were charged, the bank gained " 
        + to_string(current_commission) + " $ from account " + to_string(it->account_num));
        // unlock account
        it->account_lock.leave_writer();
    }

    // unlock accounts list
    bank->accounts_lock.leave_reader();

    // give all money taken to the bank
    bank->bank_balance_lock.enter_writer();
    bank->bank_balance += total_commission_taken;
    bank->bank_balance_lock.leave_writer();
}

void * commission_worker(void* bank_pointer)
{
    BANK* bank = (BANK*)bank_pointer;
    int commission_percent;
    
    while (!should_bank_worker_die(bank)) {
        // sleep before taking commissions
        sleep(BANK_COMMISSION_SLEEP_TIME);

        // choose commission amount
        // As this is the only place in the program that uses random numbers, we can use the non-thread-safe version of rand.
        // If any other part of the program needs random numbers, we need to change this use of rand to a thread safe function.
        commission_percent = (rand() % MAX_COMMISSION_PERCENT) + 1;
        // commission_percent = 1;

        // take commissions from all accounts
        take_commissions(bank, commission_percent);
    }
    return NULL;
}
