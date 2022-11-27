#ifndef DIPROXY_RUNSLIST_H
#define DIPROXY_RUNSLIST_H

#include <vector>
#include <string>

namespace Response {
struct RunsList {
  struct Build {
    std::string id;
    std::string url;
    std::string name;
    std::string branch;
  };

  struct Run {
    std::string id;
    std::string status;
    std::string config;
    std::string workflow;
    Build build;
  };

  std::vector<Run> runs;
};
}

#endif //DIPROXY_RUNSLIST_H
