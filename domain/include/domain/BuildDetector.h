#ifndef DIPROXY_BUILDDETECTOR_H
#define DIPROXY_BUILDDETECTOR_H

#include <vector>
#include "model/Build.h"
#include "BuildRepository.h"

class BuildDetector {
public:
  BuildDetector(BuildRepository& buildRepository, const std::string& basePath)
    : buildRepository(buildRepository),
      basePath(basePath) {};

  void detectBuilds();

private:
  BuildRepository& buildRepository;
  const std::string basePath;
};

#endif //DIPROXY_BUILDDETECTOR_H
