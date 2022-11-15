#include "domain/RunsService.h"

std::vector<std::tuple<Run, Analysis>> RunsService::listRuns() {
  auto runs = runRepository.listRuns();

  std::vector<std::tuple<Run, Analysis>> pairs;
  std::transform(runs.begin(), runs.end(), std::back_inserter(pairs), [this](const Run& run) -> std::tuple<Run, Analysis>{
    return {run, analysisRepository.get(run.analysisId)};
  });

  return pairs;
}

std::vector<std::string> RunsService::listDatasets() {
  std::vector<std::string> datasets;

  boost::filesystem::recursive_directory_iterator it(datasetsPath), end;
  while (it != end) {
    datasets.push_back(it->path().filename().string());
    ++it;
  }

  return datasets;
}

std::string RunsService::start(const std::string& analysisId, const std::string& workflow, const std::string& config, const std::string& dataset) {
  Run run{"", analysisId, workflow, config};
  run.id = runRepository.save(run);
  auto analysis = analysisRepository.get(analysisId);

  runManager.start(run, analysis, dataset);
  return run.id;
}

void RunsService::stop(const std::string& runId) {
  runManager.stop(runId);
}