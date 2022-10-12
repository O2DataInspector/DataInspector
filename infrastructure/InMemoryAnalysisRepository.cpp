#include "infrastructure/InMemoryAnalysisRepository.h"
#include <iostream>

std::string InMemoryAnalysisRepository::save(const Analysis& analysis) {
  std::cout << "InMemoryAnalysisRepository::save" << std::endl;
  analysisMutex.lock();

  auto id = analysis.id;
  if(id.empty())
    id = std::to_string(analyses.size());

  analyses.emplace_back(analysis);
  (--analyses.end())->id = id;

  analysisMutex.unlock();

  return id;
}

Analysis InMemoryAnalysisRepository::get(const std::string& analysisId) {
  std::cout << "InMemoryAnalysisRepository::get" << std::endl;
  analysisMutex.lock();
  auto analysis = analyses[std::stoi(analysisId)];
  analysisMutex.unlock();

  return analysis;
}

void InMemoryAnalysisRepository::appendLogs(const std::string& analysisId, const std::vector<std::string>& logs) {
  std::cout << "InMemoryAnalysisRepository::appendLogs" << std::endl;
  analysisMutex.lock();

  auto& analysisLogs = analyses[std::stoi(analysisId)].logs;
  analysisLogs.insert(analysisLogs.end(), logs.begin(), logs.end());

  analysisMutex.unlock();
}

void InMemoryAnalysisRepository::updateStatus(const std::string& analysisId, Analysis::BuildStatus status) {
  std::cout << "InMemoryAnalysisRepository::updateStatus" << std::endl;
  analysisMutex.lock();
  analyses[std::stoi(analysisId)].buildStatus = status;
  analysisMutex.unlock();
}
