#ifndef DIPROXY_INMEMORYANALYSISREPOSITORY_H
#define DIPROXY_INMEMORYANALYSISREPOSITORY_H

#include "domain/AnalysisRepository.h"
#include <mutex>
#include <vector>

class InMemoryAnalysisRepository: public AnalysisRepository {
public:
  ~InMemoryAnalysisRepository() override = default;

  std::string save(const Analysis& analysis) override;
  std::vector<Analysis> getAnalyses(int page, int size) override;
  Analysis get(const std::string& analysisId) override;
  void appendLogs(const std::string& analysisId, const std::vector<std::string>& logs) override;
  void updateStatus(const std::string& analysisId, Analysis::BuildStatus status) override;

private:
  std::mutex analysisMutex;
  std::vector<Analysis> analyses;
};

#endif //DIPROXY_INMEMORYANALYSISREPOSITORY_H
