#ifndef DIPROXY_ANALYSISBUILDSTATUS_H
#define DIPROXY_ANALYSISBUILDSTATUS_H

#include <string>
#include <vector>

namespace Response {
struct AnalysisBuildStatus {
  std::string status;
  std::vector<std::string> logs;
};
}

#endif //DIPROXY_ANALYSISBUILDSTATUS_H
