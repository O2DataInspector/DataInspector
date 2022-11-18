#include "domain/AnalysesService.h"
#include <iostream>

std::string AnalysesService::importAnalysis(const std::string& name, const std::string& url, const std::string& branch) {
  auto analysisId = analysisRepository.save(Analysis{"", Analysis::BuildStatus::NOT_STARTED, {}, name, url, branch});
  buildManager.build(analysisId, url, branch);

  return analysisId;
}

std::vector<Analysis> AnalysesService::getAnalyses(int page, int count) {
  return analysisRepository.getAnalyses(page, count);
}

Analysis AnalysesService::get(const std::string& analysisId) {
  return analysisRepository.get(analysisId);
}

std::vector<std::string> AnalysesService::listWorkflows(const std::string& analysisId) {
  std::vector<std::string> workflows;

  boost::filesystem::recursive_directory_iterator it(basePath + "latest-analysis-" + analysisId + "-o2/bin"), end;
  while (it != end) {
    workflows.push_back(it->path().filename().string());
    ++it;
  }

  return workflows;
}