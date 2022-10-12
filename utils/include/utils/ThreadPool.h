#ifndef DIPROXY_THREADPOOL_H
#define DIPROXY_THREADPOOL_H

#include <thread>
#include <vector>
#include <functional>
#include <deque>
#include <mutex>
#include <condition_variable>

/**
 * https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
 */

using Job = std::function<void ()>;

class ThreadPool {
public:
  ThreadPool(int size);
  void addJob(const Job& job);
  void stop();

private:
  void loop();

  volatile bool running = true;
  std::mutex queueMutex;
  std::condition_variable queueCondition;
  std::deque<Job> jobQueue;

  std::vector<std::thread> threads;
};

#endif //DIPROXY_THREADPOOL_H
