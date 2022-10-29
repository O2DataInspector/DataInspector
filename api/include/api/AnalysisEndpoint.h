#ifndef DIPROXY_ANALYSISENDPOINT_H
#define DIPROXY_ANALYSISENDPOINT_H

#include "domain/AnalysisService.h"
#include "httplib.h"

class AnalysisEndpoint {
public:
  AnalysisEndpoint(AnalysisService& analysisService) : analysisService(analysisService) {};

  void importAnalysis(const httplib::Request& input, httplib::Response& output);
  void getBuildStatus(const httplib::Request& input, httplib::Response& output);
  void listWorkflows(const httplib::Request& input, httplib::Response& output);
  void startRun(const httplib::Request& input, httplib::Response& output);
  void stopRun(const httplib::Request& input, httplib::Response& output);

private:
  AnalysisService& analysisService;
};

#endif //DIPROXY_ANALYSISENDPOINT_H
