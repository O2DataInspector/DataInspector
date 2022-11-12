#include "domain/AnalysesService.h"
#include <iostream>

std::string AnalysesService::importAnalysis(const std::string& path, const std::string& name) {
  auto analysisId = analysisRepository.save(Analysis{"", Analysis::BuildStatus::NOT_STARTED, {}, path, name});
  buildManager.build(analysisId, path);

  return analysisId;
}

std::vector<Analysis> AnalysesService::getAnalyses(int page, int count) {
  return analysisRepository.getAnalyses(page, count);
}

Analysis AnalysesService::get(const std::string& analysisId) {
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

std::vector<std::string> AnalysesService::listWorkflows(const std::string& analysisId) {
  return findWorkflows(analysisId + "/BUILD/workflows");
}