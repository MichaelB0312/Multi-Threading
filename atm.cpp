#include <unistd.h>
#include <algorithm>
#include <sstream>

#include "atm.h"
#include "logger.h"

// Sleep duration of a worker between commands (in microseconds)
#define WORKER_SLEEP_TIME (100000)
// Sleep duration inside each command (in seconds)
#define COMMAND_SLEEP_TIME (0) // TODO: Change back to 1

void* ATM::execute_command_file(void) {
    string line;
    while (getline(this->commands_file, line)) {
        // Convert line to string stream and parse command type
        stringstream line_ss;
        string command_type;
        line_ss << line;
        line_ss >> command_type;
        // Process each line here
        if (command_type[0] == 'O') this->open_account(line_ss);
        if (command_type[0] == 'D') this->deposit(line_ss);
        if (command_type[0] == 'W') this->withdrawal(line_ss);
        if (command_type[0] == 'B') this->get_balance(line_ss);
        if (command_type[0] == 'Q') this->quit_account(line_ss);
        if (command_type[0] == 'T') this->transfer(line_ss);

        // Sleep for 100 miliseconds after each command
        usleep(WORKER_SLEEP_TIME);
    }

    return NULL;
}

void ATM::open_account(stringstream& line_ss)
{
    // parse command line
    int acc_id;
    int acc_pass;
    int acc_balance;
    account new_acc;

    line_ss >> acc_id >> acc_pass >> acc_balance;

    // Lock accounts list
    this->bank->accounts_lock.enter_writer();

    // Check if the account already exists
    if (does_account_exist(this->bank, acc_id)) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account with the same id exists");
        goto cleanup;
    }

    // Create the new account
    new_acc.account_num = acc_id;
    new_acc.password = acc_pass;
    new_acc.balance = acc_balance;
    new_acc.account_lock.init();
    this->bank->accounts_list.push_back(new_acc);

    // Log creation of the new account
    log(to_string(this->id) + ": New account id is " + to_string(acc_id)
        + " with password " + to_string(acc_pass) + " and initial balance " + to_string(acc_balance));

cleanup:
    sleep(COMMAND_SLEEP_TIME); // Required sleep for each command
    this->bank->accounts_lock.leave_writer();
}

void ATM::deposit(stringstream& line_ss)
{
    // parse command line
    int acc_id;
    int acc_pass;
    int dep_amount;
    bool is_list_locked = false;
    bool is_account_locked = false;

    line_ss >> acc_id >> acc_pass >> dep_amount;

    // Lock accounts list
    this->bank->accounts_lock.enter_reader();
    is_list_locked = true;

    // Make sure the account exists
    auto it = find_if(bank->accounts_list.begin(), bank->accounts_list.end(), [acc_id](const account& acc) {
        return acc.account_num == acc_id;
        });
    if (it == bank->accounts_list.end()) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(acc_id) + " does not exist");
        goto cleanup;
    }

    // We got the right account, lock it and release the list lock
    it->account_lock.enter_writer();
    is_account_locked = true;
    this->bank->accounts_lock.leave_reader(); // TODO: Are we allowed to unlock this lock here without sleeping before leaving?
    is_list_locked = false;
    
    // Make sure password is correct
    if (acc_pass != it->password) {
        log("Error " + to_string(this->id) + ": Your transaction failed - password for account id " + to_string(acc_id) + " is incorrect");
        goto cleanup;
    }

    // Deposit sum into account
    it->balance += dep_amount;
    // Log account deposit
    log(to_string(this->id) + ": Account " + to_string(acc_id)
        + " new balance is " + to_string(it->balance) + " after " + to_string(dep_amount) + " $ was deposited");

cleanup:
    sleep(COMMAND_SLEEP_TIME); // Required sleep for each command
    if (is_list_locked) this->bank->accounts_lock.leave_reader();
    if (is_account_locked) it->account_lock.leave_writer();
}

void ATM::withdrawal(stringstream& line_ss)
{
    // parse command line
    int acc_id;
    int acc_pass;
    int withdraw_amount;
    bool is_list_locked = false;
    bool is_account_locked = false;
    
    line_ss >> acc_id >> acc_pass >> withdraw_amount;

    // Lock accounts list
    this->bank->accounts_lock.enter_reader();
    is_list_locked = true;

    // Make sure the account exists
    auto it = find_if(bank->accounts_list.begin(), bank->accounts_list.end(), [acc_id](const account& acc) {
        return acc.account_num == acc_id;
        });
    if (it == bank->accounts_list.end()) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(acc_id) + " does not exist");
        goto cleanup;
    }

    // We got the right account, lock it and release the list lock
    it->account_lock.enter_writer();
    is_account_locked = true;
    this->bank->accounts_lock.leave_reader(); // TODO: Are we allowed to unlock this lock here without sleeping before leaving?
    is_list_locked = false;
    
    // Make sure password is correct
    if (acc_pass != it->password) {
        log("Error " + to_string(this->id) + ": Your transaction failed - password for account id " + to_string(acc_id) + " is incorrect");
        goto cleanup;
    }

    // Make sure account has enough to withdraw
    if (it->balance < withdraw_amount) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(acc_id) + " balance is lower than " + to_string(withdraw_amount));
        goto cleanup;
    }

    // Withdraw sum from account
    it->balance -= withdraw_amount;
    // Log account deposit
    log(to_string(this->id) + ": Account " + to_string(acc_id)
        + " new balance is " + to_string(it->balance) + " after " + to_string(withdraw_amount) + " $ was withdrew");

cleanup:
    sleep(COMMAND_SLEEP_TIME); // Required sleep for each command
    if (is_list_locked) this->bank->accounts_lock.leave_reader();
    if (is_account_locked) it->account_lock.leave_writer();
}

void ATM::get_balance(stringstream& line_ss)
{
    // parse command line
    int acc_id;
    int acc_pass;
    bool is_list_locked = false;
    bool is_account_locked = false;

    line_ss >> acc_id >> acc_pass;

    // Lock accounts list
    this->bank->accounts_lock.enter_reader();
    is_list_locked = true;

    // Make sure the account exists
    auto it = find_if(bank->accounts_list.begin(), bank->accounts_list.end(), [acc_id](const account& acc) {
        return acc.account_num == acc_id;
        });
    if (it == bank->accounts_list.end()) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(acc_id) + " does not exist");
        goto cleanup;
    }

    // We got the right account, lock it and release the list lock
    it->account_lock.enter_reader();
    is_account_locked = true;
    this->bank->accounts_lock.leave_reader(); // TODO: Are we allowed to unlock this lock here without sleeping before leaving?
    is_list_locked = false;
    
    // Make sure password is correct
    if (acc_pass != it->password) {
        log("Error " + to_string(this->id) + ": Your transaction failed - password for account id " + to_string(acc_id) + " is incorrect");
        goto cleanup;
    }

    // Log account balance
    log(to_string(this->id) + ": Account " + to_string(acc_id)
        + " balance is " + to_string(it->balance));

cleanup:
    sleep(COMMAND_SLEEP_TIME); // Required sleep for each command
    if (is_list_locked) this->bank->accounts_lock.leave_reader();
    if (is_account_locked) it->account_lock.leave_reader();
}

void ATM::quit_account(stringstream& line_ss)
{
    // parse command line
    int acc_id;
    int acc_pass;
    int old_balance;

    line_ss >> acc_id >> acc_pass;

    // Lock accounts list
    this->bank->accounts_lock.enter_writer();

    // Make sure the account exists
    auto it = find_if(bank->accounts_list.begin(), bank->accounts_list.end(), [acc_id](const account& acc) {
        return acc.account_num == acc_id;
        });
    if (it == bank->accounts_list.end()) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(acc_id) + " does not exist");
        goto cleanup;
    }

    // We got the right account, lock it so no one can ever use it again
    // We never need to release this lock as the account will be removed (if the password is correct)
    it->account_lock.enter_writer();

    // Make sure password is correct
    if (acc_pass != it->password) {
        log("Error " + to_string(this->id) + ": Your transaction failed - password for account id " + to_string(acc_id) + " is incorrect");
        it->account_lock.leave_writer(); // This is the only case we unlock this lock. We usually destroy the account with the lock locked.
        goto cleanup;
    }

    // Remove the account from the list
    old_balance = it->balance;
    it = bank->accounts_list.erase(it);
    
    // Log deletion of the account
    log(to_string(this->id) + ": Account " + to_string(acc_id)
        + " is now closed. Balance was " + to_string(old_balance));

cleanup:
    sleep(COMMAND_SLEEP_TIME); // Required sleep for each command
    this->bank->accounts_lock.leave_writer();
}

void ATM::transfer(stringstream& line_ss)
{
    // parse command line
    int src_acc_id;
    int acc_pass;
    int dst_acc_id;
    int trans_amount;
    bool is_list_locked = false;
    bool is_src_account_locked = false;
    bool is_dst_account_locked = false;
    list<account>::iterator dst_it;

    line_ss >> src_acc_id >> acc_pass >> dst_acc_id >> trans_amount;

    // Lock accounts list
    this->bank->accounts_lock.enter_reader();
    is_list_locked = true;

    // Make sure the src account exists
    auto src_it = find_if(bank->accounts_list.begin(), bank->accounts_list.end(), [src_acc_id](const account& acc) {
        return acc.account_num == src_acc_id;
        });
    if (src_it == bank->accounts_list.end()) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(src_acc_id) + " does not exist");
        goto cleanup;
    }

    // Make sure the dst account exists
    dst_it = find_if(bank->accounts_list.begin(), bank->accounts_list.end(), [dst_acc_id](const account& acc) {
        return acc.account_num == dst_acc_id;
        });
    if (dst_it == bank->accounts_list.end()) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(dst_acc_id) + " does not exist");
        goto cleanup;
    }

    // We got the right accounts, lock them both and release the list lock
    src_it->account_lock.enter_writer();
    is_src_account_locked = true;
    dst_it->account_lock.enter_writer();
    is_dst_account_locked = true;
    this->bank->accounts_lock.leave_reader(); // TODO: Are we allowed to unlock this lock here without sleeping before leaving?
    is_list_locked = false;
    
    // Make sure password of src account is correct
    if (acc_pass != src_it->password) {
        log("Error " + to_string(this->id) + ": Your transaction failed - password for account id " + to_string(src_acc_id) + " is incorrect");
        goto cleanup;
    }

    // Make sure account has enough to withdraw
    if (src_it->balance < trans_amount) {
        log("Error " + to_string(this->id) + ": Your transaction failed - account id " + to_string(src_acc_id) + " balance is lower than " + to_string(trans_amount));
        goto cleanup;
    }

    // Make transfer
    src_it->balance -= trans_amount;
    dst_it->balance += trans_amount;
    // Log transfer
    log(to_string(this->id) + ": Transfer " + to_string(trans_amount) + " from account " + to_string(src_acc_id) 
        + " to account " + to_string(dst_acc_id) 
        + " new account balance is " + to_string(src_it->balance) 
        + " new target account balance is " + to_string(dst_it->balance));

cleanup:
    sleep(COMMAND_SLEEP_TIME); // Required sleep for each command
    if (is_list_locked) this->bank->accounts_lock.leave_reader();
    if (is_src_account_locked) src_it->account_lock.leave_writer();
    if (is_dst_account_locked) dst_it->account_lock.leave_writer();
}

bool ATM::initialize_atm(int id, char* commands_file_path, BANK* bank)
{
    // Use the provided file path as the argument
    this->id = id;
    this->commands_file = ifstream(commands_file_path);
    this->bank = bank;
    return this->commands_file.is_open();
}

void ATM::destroy_atm(void)
{
    this->commands_file.close();
}

void * create_atm_worker(void* ATM_pointer)
{
    ATM* atm = (ATM*)ATM_pointer;
    return atm->execute_command_file();
}
