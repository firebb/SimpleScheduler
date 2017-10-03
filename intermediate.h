#pragma once

#define INTERMEDIATE_SIZE 100000

class Intermediate{
private:
    pthread_mutex_t ref_lock; 
    int ref_count;
    double *result;

public:
    int worker_id;
    Intermediate(double *result, int ref_count, int worker_id) {
        this->result = result;
        this->ref_count = ref_count;
        this->worker_id = worker_id;
        pthread_mutex_init(&ref_lock, NULL);
    }

    double *get_result() {
        return this->result;
    }

    int decrement_ref() {
        pthread_mutex_lock(&ref_lock);
        int count = --ref_count;
        pthread_mutex_unlock(&ref_lock);
        return count;
    }

    ~Intermediate() {
        if (result)
            delete[] result;
    }
};
