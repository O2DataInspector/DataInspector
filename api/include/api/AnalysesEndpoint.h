#ifndef DIPROXY_ANALYSISENDPOINT_H
#define DIPROXY_ANALYSISENDPOINT_H

#include "domain/AnalysesService.h"
#include "httplib.h"
#include "response/AnalysisList.h"
#include "response/AnalysisId.h"
#include "response/AnalysisBuildStatus.h"
#include "response/WorkflowList.h"

class AnalysesEndpoint {
public:
  AnalysesEndpoint(AnalysesService& analysesService) : analysesService(analysesService) {};

  Response::AnalysisId importAnalysis(const httplib::Request& input, httplib::Response& output);
  Response::AnalysisList getAnalyses(const httplib::Request& input, httplib::Response& output);
  Response::AnalysisBuildStatus getBuildStatus(const httplib::Request& input, httplib::Response& output);
  Response::WorkflowList listWorkflows(const httplib::Request& input, httplib::Response& output);

private:
  AnalysesService& analysesService;
};

#endif //DIPROXY_ANALYSISENDPOINT_H
