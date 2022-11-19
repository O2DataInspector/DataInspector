#ifndef DIPROXY_ANALYSISSERVICE_H
#define DIPROXY_ANALYSISSERVICE_H

#include "domain/AnalysisRepository.h"
#include "domain/RunRepository.h"
#include "domain/RunManager.h"

class AnalysesService {
public:
  AnalysesService(AnalysisRepository& analysisRepository) : analysisRepository(analysisRepository) {};

  std::vector<Analysis> getAnalyses(int page, int count);
  Analysis get(const std::string& analysisId);
  std::vector<std::string> listWorkflows(const std::string& analysisId);

private:
  AnalysisRepository& analysisRepository;
};

#endif //DIPROXY_ANALYSISSERVICE_H
