#ifndef DIPROXY_RUNSSERVICE_H
#define DIPROXY_RUNSSERVICE_H

#include "domain/RunManager.h"
#include "domain/RunRepository.h"
#include "domain/BuildRepository.h"

#include <vector>
#include <tuple>

class RunsService {
public:
  RunsService(RunManager& runManager,
              RunRepository& runRepository,
              BuildRepository& buildRepository
              ) : runManager(runManager),
                  runRepository(runRepository),
                  buildRepository(buildRepository),
                  datasetsPath(std::getenv("DI_DATASETS")) {}

  std::vector<std::tuple<Run, Build>> listRuns();
  std::vector<std::string> listDatasets();
  std::string start(const std::string& buildId, const std::string& workflow, const std::string& config, const std::string& dataset);
  void stop(const std::string& runId);

private:
  RunManager& runManager;
  RunRepository& runRepository;
  BuildRepository& buildRepository;

  std::string datasetsPath;
};

#endif //DIPROXY_RUNSSERVICE_H
