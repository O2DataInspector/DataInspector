#ifndef DIPROXY_RUNSSERVICE_H
#define DIPROXY_RUNSSERVICE_H

#include "domain/RunManager.h"
#include "domain/RunRepository.h"
#include "domain/AnalysisRepository.h"

#include <vector>
#include <tuple>

class RunsService {
public:
  RunsService(RunManager& runManager,
              RunRepository& runRepository,
              AnalysisRepository& analysisRepository
              ) : runManager(runManager),
                  runRepository(runRepository),
                  analysisRepository(analysisRepository),
                  datasetsPath(std::getenv("DI_DATASETS")) {}

  std::vector<std::tuple<Run, Analysis>> listRuns();
  std::vector<std::string> listDatasets();
  std::string start(const std::string& analysisId, const std::string& workflow, const std::string& config, const std::string& dataset);
  void stop(const std::string& runId);

private:
  RunManager& runManager;
  RunRepository& runRepository;
  AnalysisRepository& analysisRepository;

  std::string datasetsPath;
};

#endif //DIPROXY_RUNSSERVICE_H
