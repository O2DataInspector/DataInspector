#include "domain/AnalysesService.h"
#include <iostream>

std::vector<Analysis> AnalysesService::getAnalyses(int page, int count) {
  return analysisRepository.getAnalyses(page, count);
}

Analysis AnalysesService::get(const std::string& analysisId) {
  return analysisRepository.get(analysisId);
}

std::vector<std::string> AnalysesService::listWorkflows(const std::string& analysisId) {
  std::vector<std::string> workflows;

  auto analysis = analysisRepository.get(analysisId);
  boost::filesystem::recursive_directory_iterator it(analysis.path + "/bin"), end;
  while (it != end) {
    workflows.push_back(it->path().filename().string());
    ++it;
  }

  return workflows;
}