#include "infrastructure/MongoAnalysisRepository.h"

std::string MongoAnalysisRepository::save(const Analysis& analysis) {
  //TODO
  throw std::runtime_error("Not implemented");
}

Analysis MongoAnalysisRepository::get(const std::string& analysisId) {
  //TODO
  throw std::runtime_error("Not implemented");
}

void MongoAnalysisRepository::appendLogs(const std::string& analysisId, const std::vector<std::string>& logs) {
  //TODO:
  //  - dodać stringi z logs do pola "logs" w bazie
  throw std::runtime_error("Not implemented");
}

void MongoAnalysisRepository::updateStatus(const std::string& analysisId, Analysis::BuildStatus status) {
  //TODO:
  //  - podmienić pole "buildStatus" na nowe
  throw std::runtime_error("Not implemented");
}
