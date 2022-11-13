#ifndef DIPROXY_RUNSENDPOINT_H
#define DIPROXY_RUNSENDPOINT_H

#include "domain/RunsService.h"
#include "httplib.h"
#include "response/RunsList.h"
#include "response/RunId.h"

class RunsEndpoint {
public:
  RunsEndpoint(RunsService& runsService) : runsService(runsService) {};

  Response::RunsList listRuns(const httplib::Request& input, httplib::Response& output);
  Response::RunId start(const httplib::Request& input, httplib::Response& output);
  void stop(const httplib::Request& input, httplib::Response& output);

private:
  RunsService& runsService;
};

#endif //DIPROXY_RUNSENDPOINT_H
