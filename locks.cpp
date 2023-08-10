#include "locks.h"

void readers_writers::init() {
    rd_count = 0;
    pthread_mutex_init(&rd_lock, nullptr);
    pthread_mutex_init(&wrt_lock, nullptr);
    // TODO: Do we really need to unlock? The man page states that a mutex starts unlocked
    pthread_mutex_unlock(&rd_lock);
    pthread_mutex_unlock(&wrt_lock);
}

void readers_writers::enter_reader() {
    pthread_mutex_lock(&rd_lock);
    rd_count++;
    if (rd_count == 1) {
        pthread_mutex_lock(&wrt_lock);
    }
    pthread_mutex_unlock(&rd_lock);
}

void readers_writers::enter_writer() {

    pthread_mutex_lock(&wrt_lock);
}

void readers_writers::leave_reader() {
    pthread_mutex_lock(&rd_lock);
    rd_count--;
    if (rd_count == 0) { //no reader for the account currently, so we can write
        pthread_mutex_unlock(&wrt_lock);
    }
    pthread_mutex_unlock(&rd_lock);
}

void readers_writers::leave_writer() {
    pthread_mutex_unlock(&wrt_lock);
}
