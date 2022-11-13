#include "api/RunsEndpoint.h"

Response::RunsList RunsEndpoint::listRuns(const httplib::Request& input, httplib::Response& output) {
  auto pairs = runsService.listRuns();

  std::vector<Response::RunsList::Run> runs;
  std::transform(pairs.begin(), pairs.end(), std::back_inserter(runs), [](const std::tuple<Run, Analysis>& pair) -> Response::RunsList::Run{
    auto& run = std::get<Run>(pair);
    auto& analysis = std::get<Analysis>(pair);

    return {run.id, run.config, run.workflow, {analysis.id, analysis.url, analysis.name, analysis.branch}};
  });

  return {runs};
}

Response::RunId RunsEndpoint::start(const httplib::Request& input, httplib::Response& output) {
  auto analysisId = input.get_header_value("analysisId");
  auto workflow = input.get_header_value("workflow");
  auto config = input.get_header_value("config");

  return {runsService.start(analysisId, workflow, config)};
}

void RunsEndpoint::stop(const httplib::Request& input, httplib::Response& output) {
  auto runId = input.get_header_value("runId");
  runsService.stop(runId);
}
