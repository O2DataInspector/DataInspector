#ifndef DIPROXY_MONGORUNREPOSITORY_H
#define DIPROXY_MONGORUNREPOSITORY_H

#include "domain/RunRepository.h"
#include <mutex>
#include <vector>
#include "mongoc.h"

class MongoRunRepository: public RunRepository {
public:
  MongoRunRepository(mongoc_client_pool_t* pool): pool(pool) {};

  std::string save(const Run& run) override;
  Run get(const std::string& runId) override;
  std::vector<Run> listRuns() override;
  void finish(const std::string& runId) override;

private:
  mongoc_client_pool_t* pool;
};

#endif //DIPROXY_MONGORUNREPOSITORY_H
