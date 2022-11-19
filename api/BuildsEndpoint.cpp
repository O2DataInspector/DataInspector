#include "api/BuildsEndpoint.h"
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

Response::BuildList BuildsEndpoint::getAnalyses(const httplib::Request& input, httplib::Response& output) {
  auto page = lexicalCastOrDefault<int>(input.get_header_value("page"), 0);
  auto count = lexicalCastOrDefault<int>(input.get_header_value("count"), 10);

  auto analyses = analysesService.getBuilds(page, count);

  std::vector<Response::BuildList::Build> response{};
  std::transform(analyses.begin(), analyses.end(), std::back_inserter(response), [](const Build& build) {
    return Response::BuildList::Build{
      build.id,
      build.url,
      build.name
    };
  });

  return {response};
}

Response::WorkflowList BuildsEndpoint::listWorkflows(const httplib::Request& input, httplib::Response& output) {
  auto buildId = input.get_header_value("buildId");
  auto workflows = analysesService.listWorkflows(buildId);

  return {workflows};
}
