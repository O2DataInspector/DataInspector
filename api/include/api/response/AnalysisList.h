#ifndef DIPROXY_ANALYSISLIST_H
#define DIPROXY_ANALYSISLIST_H

namespace Response {
struct AnalysisList {
  struct Analysis {
    std::string id;
    std::string url;
    std::string name;
    std::string branch;
  };

  std::vector<Analysis> analyses;
};
}

#endif //DIPROXY_ANALYSISLIST_H
