#include "domain/AnalysisService.h"
#include <iostream>

std::string AnalysisService::importAnalysis(const std::string& path, const std::string& name) {
  auto analysisId = analysisRepository.save(Analysis{"", Analysis::BuildStatus::NOT_STARTED, {}, path, name});
  buildManager.build(analysisId, path);

  return analysisId;
}

Analysis AnalysisService::get(const std::string& analysisId) {
  return analysisRepository.get(analysisId);
}

std::vector<std::string> findWorkflows(const std::string& buildPath) {
  std::vector<std::string> workflows;
  auto analysesPath = std::string{std::getenv("ANALYSES_PATH")};

  boost::filesystem::recursive_directory_iterator it(analysesPath + buildPath), end;
  while (it != end) {
    workflows.push_back(it->path().filename().string());
    ++it;
  }

  return workflows;
}

std::vector<std::string> AnalysisService::listWorkflows(const std::string& analysisId) {
  return findWorkflows("/" + analysisId + "/BUILD/workflows");
}

std::string AnalysisService::startAnalysis(const std::string& analysisId, const std::string& workflow, const std::string& config) {
  auto runId = runRepository.save(Run{"", analysisId, workflow, config});
  auto analysis = analysisRepository.get(analysisId);
  auto run = runRepository.get(runId);
  runManager.start(run, analysis, config);

  return runId;
}

void AnalysisService::sopAnalysis(const std::string& runId) {
  runManager.stop(runId);
}
