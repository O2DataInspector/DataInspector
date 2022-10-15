#include "utils/ThreadPool.h"

ThreadPool::ThreadPool(int size) {
  for(int i=0; i<size; i++) {
    threads.emplace_back(std::thread{[this]() { loop(); }});
  }
}

void ThreadPool::addJob(const Job &job) {
  queueMutex.lock();
  jobQueue.push_back(job);
  queueMutex.unlock();

  queueCondition.notify_one();
}

void ThreadPool::loop() {
  while (running) {
    Job job;
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      queueCondition.wait(lock, [this]() { return !jobQueue.empty() || !running; });

      if(!running)
        return;

      job = jobQueue.front();
      jobQueue.pop_front();
    }

    job();
  }
}

void ThreadPool::stop() {
  running = false;
}
