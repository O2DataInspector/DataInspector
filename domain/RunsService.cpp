#include "domain/RunsService.h"

std::vector<std::tuple<Run, Build>> RunsService::listRuns() {
  auto runs = runRepository.listRuns();

  std::vector<std::tuple<Run, Build>> pairs;
  std::transform(runs.begin(), runs.end(), std::back_inserter(pairs), [this](const Run& run) -> std::tuple<Run, Build>{
    return {run, buildRepository.get(run.buildId)};
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

std::string RunsService::start(const std::string& buildId, const std::string& workflow, const std::string& config, const std::optional<std::string>& dataset) {
  Run run{"", Run::Status::STARTING, buildId, workflow, config};
  run.id = runRepository.save(run);
  auto build = buildRepository.get(buildId);

  runManager.start(run, build, dataset);
  return run.id;
}

void RunsService::stop(const std::string& runId) {
  runManager.stop(runId);
}