#include "api/AnalysesEndpoint.h"
#include "rapidjson/document.h"
#include "httplib.h"

#include <boost/lexical_cast.hpp>

Response::AnalysisId AnalysesEndpoint::importAnalysis(const httplib::Request& input, httplib::Response& output) {
  auto name = input.get_header_value("name");
  auto url = input.get_header_value("url");
  auto branch = input.get_header_value("branch");

  auto analysisId = analysesService.importAnalysis(name, url, branch);
  return {analysisId};
}

template <typename T>
T lexicalCastOrDefault(const std::string& src, T def) {
  try {
    return boost::lexical_cast<T>(src);
  } catch (boost::bad_lexical_cast const& e) {
    return def;
  }
}

std::string toString(const Analysis::BuildStatus buildStatus) {
  std::string status;
  switch (buildStatus) {
    case Analysis::BuildStatus::ERROR: {
      status = "ERROR";
      break;
    }
    case Analysis::BuildStatus::OK: {
      status = "OK";
      break;
    }
    case Analysis::BuildStatus::IN_PROGRESS: {
      status = "IN_PROGRESS";
      break;
    }
    case Analysis::BuildStatus::NOT_STARTED: {
      status = "NOT_STARTED";
      break;
    }
  }

  return status;
}

Response::AnalysisList AnalysesEndpoint::getAnalyses(const httplib::Request& input, httplib::Response& output) {
  auto page = lexicalCastOrDefault<int>(input.get_header_value("page"), 0);
  auto count = lexicalCastOrDefault<int>(input.get_header_value("count"), 10);

  auto analyses = analysesService.getAnalyses(page, count);

  std::vector<Response::AnalysisList::Analysis> response{};
  std::transform(analyses.begin(), analyses.end(), std::back_inserter(response), [](const Analysis& analysis) {
    return Response::AnalysisList::Analysis{
      analysis.id,
      toString(analysis.buildStatus),
      analysis.url,
      analysis.name
    };
  });

  return {response};
}

Response::AnalysisBuildStatus AnalysesEndpoint::getBuildStatus(const httplib::Request& input, httplib::Response& output) {
  auto analysisId = input.get_header_value("analysisId");
  auto analysis = analysesService.get(analysisId);

  auto status = toString(analysis.buildStatus);
  return Response::AnalysisBuildStatus{status, analysis.logs};
}

Response::WorkflowList AnalysesEndpoint::listWorkflows(const httplib::Request& input, httplib::Response& output) {
  auto analysisId = input.get_header_value("analysisId");
  auto workflows = analysesService.listWorkflows(analysisId);

  return {workflows};
}
