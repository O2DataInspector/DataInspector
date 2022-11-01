#include "api/DevicesEndpoint.h"
#include "boost/algorithm/string.hpp"

void DevicesEndpoint::selectDevices(const httplib::Request& input, httplib::Response& output) {
  std::vector<std::string> devicesNames{};
  std::string devicesString = input.get_header_value("devices");
  boost::split(devicesNames, devicesString, boost::is_any_of(","));

  std::string runId = input.get_header_value("runId");
  devicesService.intercept(runId, devicesNames);
}

void DevicesEndpoint::selectAll(const httplib::Request& input, httplib::Response& output) {
  std::string runId = input.get_header_value("runId");
  devicesService.intercept(runId);
}

void DevicesEndpoint::unselectAll(const httplib::Request& input, httplib::Response& output) {
  std::string runId = input.get_header_value("runId");
  devicesService.stopInterception(runId);
}

Response::DeviceList DevicesEndpoint::getDevices(const httplib::Request& input, httplib::Response& output) {
  std::string runId = input.get_header_value("runId");
  return {devicesService.getDevices(runId)};
}
