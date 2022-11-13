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
                  analysisRepository(analysisRepository) {}

  std::vector<std::tuple<Run, Analysis>> listRuns();
  std::string start(const std::string& analysisId, const std::string& workflow, const std::string& config);
  void stop(const std::string& runId);

private:
  RunManager& runManager;
  RunRepository& runRepository;
  AnalysisRepository& analysisRepository;
};

#endif //DIPROXY_RUNSSERVICE_H
