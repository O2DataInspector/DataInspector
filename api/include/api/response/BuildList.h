#ifndef DIPROXY_ANALYSISLIST_H
#define DIPROXY_ANALYSISLIST_H

namespace Response {
struct BuildList {
  struct Build {
    std::string id;
    std::string url;
    std::string name;
    std::string branch;
  };

  std::vector<Build> builds;
};
}

#endif //DIPROXY_ANALYSISLIST_H
