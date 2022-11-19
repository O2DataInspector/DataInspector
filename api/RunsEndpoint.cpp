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
  std::transform(pairs.begin(), pairs.end(), std::back_inserter(runs), [](const std::tuple<Run, Analysis>& pair) -> Response::RunsList::Run{
    auto& run = std::get<Run>(pair);
    auto& analysis = std::get<Analysis>(pair);

    return {run.id, toString(run.status), run.config, run.workflow, {analysis.id, analysis.url, analysis.name, analysis.branch}};
  });

  return {runs};
}

Response::DatasetList RunsEndpoint::listDatasets(const httplib::Request& input, httplib::Response& output) {
  return {runsService.listDatasets()};
}

Response::RunId RunsEndpoint::start(const httplib::Request& input, httplib::Response& output) {
  auto analysisId = input.get_header_value("analysisId");
  auto workflow = input.get_header_value("workflow");
  auto config = input.get_header_value("config");
  auto dataset = input.get_header_value("dataset");

  return {runsService.start(analysisId, workflow, config, dataset)};
}

void RunsEndpoint::stop(const httplib::Request& input, httplib::Response& output) {
  auto runId = input.get_header_value("runId");
  runsService.stop(runId);
}
