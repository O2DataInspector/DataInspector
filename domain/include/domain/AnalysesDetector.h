#ifndef DIPROXY_ANALYSESDETECTOR_H
#define DIPROXY_ANALYSESDETECTOR_H

#include <vector>
#include "model/Analysis.h"
#include "AnalysisRepository.h"

class AnalysesDetector {
public:
  AnalysesDetector(AnalysisRepository& analysisRepository, const std::string& basePath)
    : analysisRepository(analysisRepository),
      basePath(basePath) {};

  void detectAnalyses();

private:
  AnalysisRepository& analysisRepository;
  const std::string basePath;
};

#endif //DIPROXY_ANALYSESDETECTOR_H
