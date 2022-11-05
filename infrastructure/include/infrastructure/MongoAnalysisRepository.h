#ifndef DIPROXY_MONGOANALYSISREPOSITORY_H
#define DIPROXY_MONGOANALYSISREPOSITORY_H

#include "domain/AnalysisRepository.h"
#include <vector>
#include "mongoc.h"

class MongoAnalysisRepository: public AnalysisRepository {
public:
  MongoAnalysisRepository(mongoc_client_pool_t* pool): pool(pool) {};

  std::string save(const Analysis& analysis) override;
  Analysis get(const std::string& analysisId) override;
  void appendLogs(const std::string& analysisId, const std::vector<std::string>& logs) override;
  void updateStatus(const std::string& analysisId, Analysis::BuildStatus status) override;

private:
  mongoc_client_pool_t* pool;
};

#endif //DIPROXY_MONGOANALYSISREPOSITORY_H
