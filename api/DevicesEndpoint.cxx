#include "api/DevicesEndpoint.h"
#include "boost/algorithm/string.hpp"

void DevicesEndpoint::selectDevices(const httplib::Request& input, httplib::Response& output) {
  std::vector<std::string> devicesNames{};
  std::string devicesString = input.get_header_value("devices");
  boost::split(devicesNames, devicesString, boost::is_any_of(","));

  std::string analysisId = input.get_header_value("analysis-id");
  devicesService.intercept(analysisId, devicesNames);
}

void DevicesEndpoint::selectAll(const httplib::Request& input, httplib::Response& output) {
  std::string analysisId = input.get_header_value("analysis-id");
  devicesService.intercept(analysisId);
}

void DevicesEndpoint::unselectAll(const httplib::Request& input, httplib::Response& output) {
  std::string analysisId = input.get_header_value("analysis-id");
  devicesService.stopInterception(analysisId);
}

void DevicesEndpoint::getDevices(const httplib::Request& input, httplib::Response& output) {
  std::string analysisId = input.get_header_value("analysis-id");

  std::vector<std::string> deviceNames{};
  auto deviceList = devicesService.getDevices(analysisId);
  std::transform(deviceList.begin(), deviceList.end(), std::back_inserter(deviceNames), [](const Device& device) -> std::string{
    return "\"" + device.name + "\"";
  });

  std::string response = "{\"names\":[" + boost::join(deviceNames, ",") + "]}";
  output.set_content(response, "application/json");
}
