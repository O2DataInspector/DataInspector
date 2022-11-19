#ifndef DIPROXY_BUILDREPOSITORY_H
#define DIPROXY_BUILDREPOSITORY_H

#include "domain/model/Build.h"
#include <optional>

class BuildRepository {
public:
  virtual ~BuildRepository() {};

  virtual std::string save(const Build& build) = 0;
  virtual std::vector<Build> getAnalyses(int page, int size) = 0;
  virtual Build get(const std::string& buildId) = 0;
  virtual std::optional<Build> getByName(const std::string& name) = 0;
};

#endif //DIPROXY_BUILDREPOSITORY_H
