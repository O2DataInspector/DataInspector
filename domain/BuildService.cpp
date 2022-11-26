#include "domain/BuildService.h"
#include <iostream>

std::vector<Build> BuildService::getBuilds(int page, int count) {
  return buildRepository.getAnalyses(page, count);
}

Build BuildService::get(const std::string& buildId) {
  return buildRepository.get(buildId);
}

std::vector<std::string> BuildService::listWorkflows(const std::string& buildId) {
  std::vector<std::string> workflows;

  auto build = buildRepository.get(buildId);
  boost::filesystem::recursive_directory_iterator it(basePath + "/" + build.path + "/bin"), end;
  while (it != end) {
    workflows.push_back(it->path().filename().string());
    ++it;
  }

  return workflows;
}