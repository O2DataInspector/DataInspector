#ifndef DIPROXY_MONGOBUILDREPOSITORY_H
#define DIPROXY_MONGOBUILDREPOSITORY_H

#include "domain/BuildRepository.h"
#include <vector>
#include "mongoc.h"

class MongoBuildRepository: public BuildRepository {
public:
  MongoBuildRepository(mongoc_client_pool_t* pool): pool(pool) {};
  ~MongoBuildRepository() override {};

  std::string save(const Build& build) override;
  std::vector<Build> getAnalyses(int page, int count) override;
  Build get(const std::string& buildId) override;
  std::optional<Build> getByName(const std::string& name) override;

private:
  mongoc_client_pool_t* pool;
};

#endif //DIPROXY_MONGOBUILDREPOSITORY_H
