#ifndef DIPROXY_ANALYSISREPOSITORY_H
#define DIPROXY_ANALYSISREPOSITORY_H

#include "domain/model/Analysis.h"
#include <optional>

class AnalysisRepository {
public:
  virtual ~AnalysisRepository() {};

  virtual std::string save(const Analysis& analysis) = 0;
  virtual std::vector<Analysis> getAnalyses(int page, int size) = 0;
  virtual Analysis get(const std::string& analysisId) = 0;
  virtual std::optional<Analysis> getByName(const std::string& name) = 0;
};

#endif //DIPROXY_ANALYSISREPOSITORY_H
