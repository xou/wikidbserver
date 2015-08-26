#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>

using namespace std;

/**
 * Queue that blocks both on reads and on writes, if queue is empty or at max_queue_size.
 * Additionally, contains a signalling mechanism to notify consumer threads.
 *
 * TODO change implementation to ringbuffer
 */
template<typename T>
class ProducerConsumerQueue {
  private:
    queue<T> queue_;
    mutex mutex_;
    condition_variable cond_;
    bool terminate_consumer = false;
    const size_t max_queue_size;

  public:
    const ProducerConsumerQueue& operator=(ProducerConsumerQueue &) = delete;
    ProducerConsumerQueue(const ProducerConsumerQueue &other) = delete;
    ProducerConsumerQueue(ProducerConsumerQueue &&other) = default;
    ProducerConsumerQueue(size_t max_queue_size = 4096) : max_queue_size(max_queue_size) {}


    void terminate_consumers() {
      unique_lock<mutex> lock(mutex_);
      terminate_consumer = true;
      cond_.notify_all();
    }

    size_t size() {
      unique_lock<mutex> lock(mutex_);
      return queue_.size();
    }

    void push(const T& obj) {
      unique_lock<mutex> lock(mutex_);
      // TODO is this lambda threadsafe?
      cond_.wait(lock, [this]{return this->queue_.size() < max_queue_size;});
      queue_.push(obj); 
      cond_.notify_all();
    }

    // returns false if no item could be extracted and the producer has requested shutdown.
    // returns true and writes the next item in 'out' otherwise.
    bool pop(T &out) {
      unique_lock<mutex> lock(mutex_);
      cond_.wait(lock, [this]{return (!this->queue_.empty() || this->terminate_consumer);});
      if (queue_.empty()) {
        return false;
      }
      out = queue_.front();
      queue_.pop();
      cond_.notify_all();
      return true;
    }
};

