#ifndef DIPROXY_ANALYSIS_H
#define DIPROXY_ANALYSIS_H

#include <string>
#include <vector>

struct Analysis {
  enum class BuildStatus: uint8_t {
    NOT_STARTED = 0,
    OK = 1,
    ERROR = 2,
    IN_PROGRESS = 3
  };

  std::string id;
  BuildStatus buildStatus;
  std::vector<std::string> logs;

  std::string path;
  std::string name;
};

#endif //DIPROXY_ANALYSIS_H
