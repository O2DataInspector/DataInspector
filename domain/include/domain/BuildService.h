#ifndef DIPROXY_ANALYSISSERVICE_H
#define DIPROXY_ANALYSISSERVICE_H

#include "domain/BuildRepository.h"
#include "domain/RunRepository.h"
#include "domain/RunManager.h"

class BuildService {
public:
  BuildService(BuildRepository& buildRepository) : buildRepository(buildRepository) {};

  std::vector<Build> getBuilds(int page, int count);
  Build get(const std::string& buildId);
  std::vector<std::string> listWorkflows(const std::string& buildId);

private:
  BuildRepository& buildRepository;
};

#endif //DIPROXY_ANALYSISSERVICE_H
