#ifndef DIPROXY_WORKFLOWLIST_H
#define DIPROXY_WORKFLOWLIST_H

#include <string>
#include <vector>

namespace Response {
struct WorkflowList {
  std::vector<std::string> workflows;
};
}

#endif //DIPROXY_WORKFLOWLIST_H
