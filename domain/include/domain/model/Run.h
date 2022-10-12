#ifndef DIPROXY_RUN_H
#define DIPROXY_RUN_H

#include <string>

struct Run {
  std::string id;
  std::string analysisId;
  std::string workflow;
  std::string config;
};

#endif //DIPROXY_RUN_H
