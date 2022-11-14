#ifndef DIPROXY_RUNREPOSITORY_H
#define DIPROXY_RUNREPOSITORY_H

#include "domain/model/Run.h"
#include <vector>

class RunRepository {
public:
  RunRepository() {};

  virtual std::string save(const Run& run) = 0;
  virtual Run get(const std::string& runId) = 0;
  virtual std::vector<Run> listRuns() = 0;
  virtual void finish(const std::string& runId) = 0;
};

#endif //DIPROXY_RUNREPOSITORY_H
