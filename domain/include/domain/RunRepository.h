#ifndef DIPROXY_RUNREPOSITORY_H
#define DIPROXY_RUNREPOSITORY_H

#include "domain/model/Run.h"
#include <vector>
#include <stdexcept>

struct RunNotFound : std::runtime_error {
  RunNotFound(const std::string& what) : std::runtime_error(what) {};
};

struct RunNotSaved : std::runtime_error {
  RunNotSaved(const std::string& what) : std::runtime_error(what) {};
};

class RunRepository {
public:
  RunRepository() {};

  virtual std::string save(const Run& run) = 0;
  virtual Run get(const std::string& runId) = 0;
  virtual std::vector<Run> listRuns() = 0;
  virtual void updateStatus(const std::string& runId, Run::Status status) = 0;
};

#endif //DIPROXY_RUNREPOSITORY_H
