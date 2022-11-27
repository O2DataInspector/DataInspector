#include "api/RunsEndpoint.h"

std::string toString(Run::Status status) {
  std::string value;
  switch (status) {
    case Run::Status::STARTING: {
      value = "STARTING";
      break;
    }
    case Run::Status::RUNNING: {
      value = "RUNNING";
      break;
    }
    case Run::Status::FINISHED: {
      value = "FINISHED";
      break;
    }
    case Run::Status::FAILED: {
      value = "FAILED";
      break;
    }
  }

  return value;
}

Response::RunsList RunsEndpoint::listRuns(const httplib::Request& input, httplib::Response& output) {
  auto pairs = runsService.listRuns();

  std::vector<Response::RunsList::Run> runs;
  std::transform(pairs.begin(), pairs.end(), std::back_inserter(runs), [](const std::tuple<Run, Build>& pair) -> Response::RunsList::Run{
    auto& run = std::get<Run>(pair);
    auto& build = std::get<Build>(pair);

    return {run.id, toString(run.status), run.config, run.workflow, {build.id, build.url, build.name, build.branch}};
  });

  return {runs};
}

Response::DatasetList RunsEndpoint::listDatasets(const httplib::Request& input, httplib::Response& output) {
  return {runsService.listDatasets()};
}

Response::RunId RunsEndpoint::start(const httplib::Request& input, httplib::Response& output) {
  auto buildId = input.get_header_value("buildId");
  auto workflow = input.get_header_value("workflow");
  auto config = input.get_header_value("config");
  std::optional<std::string> dataset;
  if(input.has_header("dataset"))
    dataset = input.get_header_value("dataset");

  if(buildId.empty() || workflow.empty())
    throw StartRunBadRequest{"Not enough arguments to start a run"};

  return {runsService.start(buildId, workflow, config, dataset)};
}

void RunsEndpoint::stop(const httplib::Request& input, httplib::Response& output) {
  auto runId = input.get_header_value("runId");
  runsService.stop(runId);
}
