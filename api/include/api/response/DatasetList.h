#ifndef DIPROXY_DATASETLIST_H
#define DIPROXY_DATASETLIST_H

#include <string>
#include <vector>

namespace Response {
struct DatasetList {
  std::vector<std::string> datasets;
};
}

#endif //DIPROXY_DATASETLIST_H
