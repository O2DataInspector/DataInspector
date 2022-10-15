#ifndef DIPROXY_ANALYSISREPOSITORY_H
#define DIPROXY_ANALYSISREPOSITORY_H

#include "domain/model/Analysis.h"

class AnalysisRepository {
public:
  AnalysisRepository() {};

  virtual std::string save(const Analysis& analysis) = 0;
  virtual Analysis get(const std::string& analysisId) = 0;
  virtual void appendLogs(const std::string& analysisId, const std::vector<std::string>& logs) = 0;
  virtual void updateStatus(const std::string& analysisId, Analysis::BuildStatus status) = 0;
};

#endif //DIPROXY_ANALYSISREPOSITORY_H
