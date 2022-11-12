#ifndef DIPROXY_ANALYSISSERVICE_H
#define DIPROXY_ANALYSISSERVICE_H

#include "domain/AnalysisRepository.h"
#include "domain/RunRepository.h"
#include "domain/BuildManager.h"
#include "domain/RunManager.h"

class AnalysesService {
public:
  AnalysesService(BuildManager& buildManager, AnalysisRepository& analysisRepository):
          buildManager(buildManager),
          analysisRepository(analysisRepository) {};

  std::string importAnalysis(const std::string& path, const std::string& name);
  std::vector<Analysis> getAnalyses(int page, int count);
  Analysis get(const std::string& analysisId);
  std::vector<std::string> listWorkflows(const std::string& analysisId);

private:
  BuildManager& buildManager;
  AnalysisRepository& analysisRepository;
};

#endif //DIPROXY_ANALYSISSERVICE_H
