#ifndef DIPROXY_RUN_H
#define DIPROXY_RUN_H

#include <string>

struct Run {
  enum Status: int32_t {
    STARTING = 0,
    RUNNING = 1,
    FAILED = 2,
    FINISHED = 3
  };

  std::string id;
  Status status;
  std::string buildId;
  std::string workflow;
  std::string config;
};

#endif //DIPROXY_RUN_H
