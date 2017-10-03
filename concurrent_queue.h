/** @file console.h
 *  @brief A thread-safe queue to send and receive messages. 
 *
 */
#pragma once

#include <vector>

template <class T>
class ConcurrentQueue {
 private:
  std::vector<T> storage;
  pthread_mutex_t queue_lock;
  pthread_cond_t queue_cond;

 public:
  ConcurrentQueue() {
    pthread_cond_init(&queue_cond, NULL);
    pthread_mutex_init(&queue_lock, NULL);
  }

  T poll() {
    pthread_mutex_lock(&queue_lock);
    while (storage.size() == 0) {
      pthread_cond_wait(&queue_cond, &queue_lock);
    }

    T item = storage.front();
    storage.erase(storage.begin());

    pthread_mutex_unlock(&queue_lock);
    return item;
  }

  void push(const T& item) {
    pthread_mutex_lock(&queue_lock);
    storage.push_back(item);
    pthread_mutex_unlock(&queue_lock);
    pthread_cond_signal(&queue_cond);
  }
};
