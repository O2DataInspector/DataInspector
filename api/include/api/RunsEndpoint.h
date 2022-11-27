#ifndef DIPROXY_RUNSENDPOINT_H
#define DIPROXY_RUNSENDPOINT_H

#include "domain/RunsService.h"
#include "httplib.h"
#include "response/RunsList.h"
#include "response/RunId.h"
#include "api/response/DatasetList.h"

struct StartRunBadRequest : std::runtime_error {
  StartRunBadRequest(const std::string& what) : std::runtime_error(what) {};
};

class RunsEndpoint {
public:
  RunsEndpoint(RunsService& runsService) : runsService(runsService) {};

  Response::RunsList listRuns(const httplib::Request& input, httplib::Response& output);
  Response::DatasetList listDatasets(const httplib::Request& input, httplib::Response& output);
  Response::RunId start(const httplib::Request& input, httplib::Response& output);
  void stop(const httplib::Request& input, httplib::Response& output);

private:
  RunsService& runsService;
};

#endif //DIPROXY_RUNSENDPOINT_H
