#ifndef DIPROXY_INMEMORYRUNREPOSITORY_H
#define DIPROXY_INMEMORYRUNREPOSITORY_H

#include "domain/RunRepository.h"
#include <mutex>
#include <vector>

class InMemoryRunRepository: public RunRepository {
public:
  InMemoryRunRepository() {};

  std::string save(const Run& run) override;
  Run get(const std::string& runId) override;
  std::vector<Run> listRuns() override;

private:
  std::mutex runMutex;
  std::vector<Run> runs;
};

#endif //DIPROXY_INMEMORYRUNREPOSITORY_H
