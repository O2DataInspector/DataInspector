#ifndef DIPROXY_ANALYSISENDPOINT_H
#define DIPROXY_ANALYSISENDPOINT_H

#include "domain/BuildService.h"
#include "httplib.h"
#include "response/BuildList.h"
#include "response/BuildId.h"
#include "response/WorkflowList.h"

class BuildsEndpoint {
public:
  BuildsEndpoint(BuildService& analysesService) : analysesService(analysesService) {};

  Response::BuildId importBuild(const httplib::Request& input, httplib::Response& output);
  Response::BuildList getBuilds(const httplib::Request& input, httplib::Response& output);
  Response::WorkflowList listWorkflows(const httplib::Request& input, httplib::Response& output);

private:
  BuildService& analysesService;
};

#endif //DIPROXY_ANALYSISENDPOINT_H
