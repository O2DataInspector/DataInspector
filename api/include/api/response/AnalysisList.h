#ifndef DIPROXY_ANALYSISLIST_H
#define DIPROXY_ANALYSISLIST_H

namespace Response {
struct AnalysisList {
  struct Analysis {
    std::string id;
    std::string buildStatus;
    std::string path;
    std::string name;
  };

  std::vector<Analysis> analyses;
};
}

#endif //DIPROXY_ANALYSISLIST_H
