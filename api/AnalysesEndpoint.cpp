#include "api/AnalysesEndpoint.h"
#include "rapidjson/document.h"
#include "httplib.h"

#include <boost/lexical_cast.hpp>

template <typename T>
T lexicalCastOrDefault(const std::string& src, T def) {
  try {
    return boost::lexical_cast<T>(src);
  } catch (boost::bad_lexical_cast const& e) {
    return def;
  }
}

Response::AnalysisList AnalysesEndpoint::getAnalyses(const httplib::Request& input, httplib::Response& output) {
  auto page = lexicalCastOrDefault<int>(input.get_header_value("page"), 0);
  auto count = lexicalCastOrDefault<int>(input.get_header_value("count"), 10);

  auto analyses = analysesService.getAnalyses(page, count);

  std::vector<Response::AnalysisList::Analysis> response{};
  std::transform(analyses.begin(), analyses.end(), std::back_inserter(response), [](const Analysis& analysis) {
    return Response::AnalysisList::Analysis{
      analysis.id,
      analysis.url,
      analysis.name
    };
  });

  return {response};
}

Response::WorkflowList AnalysesEndpoint::listWorkflows(const httplib::Request& input, httplib::Response& output) {
  auto analysisId = input.get_header_value("analysisId");
  auto workflows = analysesService.listWorkflows(analysisId);

  return {workflows};
}
