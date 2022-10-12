#include "api/AnalysisEndpoint.h"
#include "rapidjson/document.h"
#include "httplib.h"

void AnalysisEndpoint::importAnalysis(const httplib::Request& input, httplib::Response& output) {
  rapidjson::Document document;
  document.Parse(input.body.c_str());

  auto& name = document.FindMember("name")->value;
  auto& path = document.FindMember("path")->value;

  auto analysisId = analysisService.importAnalysis(std::string{path.GetString(), path.GetStringLength()}, std::string{name.GetString(), name.GetStringLength()});

  output.set_content("{\"id\":\"" + analysisId + "\"}", "application/json");
}

void AnalysisEndpoint::getBuildStatus(const httplib::Request& input, httplib::Response& output) {
  auto analysisId = input.get_param_value("analysisId");

  auto analysis = analysisService.get(analysisId);
  if(analysis.buildStatus == Analysis::BuildStatus::OK) {
    output.set_content("{\"status\":\"OK\"}", "application/json");
  } else if (analysis.buildStatus == Analysis::BuildStatus::ERROR) {
    output.set_content("{\"status\":\"ERROR\"}", "application/json");
  } else {
    std::vector<std::string> jsonLogs;
    if(!analysis.logs.empty()) {
      std::transform(analysis.logs.begin(), analysis.logs.end(), std::back_inserter(jsonLogs), [](const std::string& log) {
        return "\"" + log.substr(0, log.size()-1) + "\"";
      });
    }
    output.set_content("{\"status\":\"OTHER\",\"logs\":[" + boost::join(jsonLogs, ",") + "]}", "application/json");
  }
}

void AnalysisEndpoint::listWorkflows(const httplib::Request& input, httplib::Response& output) {
  auto analysisId = input.get_param_value("analysisId");
  auto workflows = analysisService.listWorkflows(analysisId);

  std::vector<std::string> jsonNames;
  std::transform(workflows.begin(), workflows.end(), std::back_inserter(jsonNames), [](const std::string& workflow) {
    return "\"" + workflow + "\"";
  });

  output.set_content("{\"workflows\":[" + boost::join(jsonNames, ",") + "]}", "application/json");
}

void AnalysisEndpoint::startAnalysis(const httplib::Request& input, httplib::Response& output) {
  rapidjson::Document document;
  document.Parse(input.body.c_str());

  auto& analysisId = document.FindMember("analysisId")->value;
  auto& workflow = document.FindMember("workflow")->value;
  auto& config = document.FindMember("config")->value;

  auto runId = analysisService.startAnalysis(std::string{analysisId.GetString(), analysisId.GetStringLength()}, std::string{workflow.GetString(), workflow.GetStringLength()}, std::string{config.GetString(), config.GetStringLength()});

  output.set_content("{\"id\":\"" + runId + "\"}", "application/json");
}

void AnalysisEndpoint::stopAnalysis(const httplib::Request& input, httplib::Response& output) {
  auto runId = input.get_param_value("runId");
  analysisService.sopAnalysis(runId);
}
