#ifndef DIPROXY_ANALYSISSERVICE_H
#define DIPROXY_ANALYSISSERVICE_H

#include "domain/AnalysisRepository.h"
#include "domain/RunRepository.h"
#include "domain/BuildManager.h"
#include "domain/RunManager.h"

class AnalysisService {
public:
  AnalysisService(BuildManager& buildManager,
                  RunManager& runManager,
                  AnalysisRepository& analysisRepository,
                  RunRepository& runRepository
                  ):
                    runManager(runManager),
                    buildManager(buildManager),
                    analysisRepository(analysisRepository),
                    runRepository(runRepository) {};

  std::string importAnalysis(const std::string& path, const std::string& name);
  Analysis get(const std::string& analysisId);
  std::vector<std::string> listWorkflows(const std::string& analysisId);
  std::string startAnalysis(const std::string& analysisId, const std::string& workflow, const std::string& config);
  void sopAnalysis(const std::string& runId);

private:
  RunManager& runManager;
  BuildManager& buildManager;
  AnalysisRepository& analysisRepository;
  RunRepository& runRepository;
};

#endif //DIPROXY_ANALYSISSERVICE_H
