#include "infrastructure/InMemoryRunRepository.h"
#include <iostream>

std::string InMemoryRunRepository::save(const Run& run) {
  std::cout << "InMemoryRunRepository::save" << std::endl;
  runMutex.lock();

  auto id = run.id;
  if(id.empty())
    id = std::to_string(runs.size());

  runs.emplace_back(run);
  (--runs.end())->id = id;

  runMutex.unlock();

  return id;
}

Run InMemoryRunRepository::get(const std::string& runId) {
  std::cout << "InMemoryRunRepository::get" << std::endl;
  runMutex.lock();
  auto run = runs[std::stoi(runId)];
  runMutex.unlock();

  return run;
}

std::vector<Run> InMemoryRunRepository::getAnalysisRuns(const std::string& analysisId) {
  std::cout << "InMemoryRunRepository::getAnalysisRuns" << std::endl;
  std::vector<Run> analysisRuns{};

  runMutex.lock();
  for(auto& run : runs) {
    if(run.analysisId != analysisId)
      continue;

    analysisRuns.emplace_back(run);
  }
  runMutex.unlock();

  return analysisRuns;
}
