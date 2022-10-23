#include "api/DevicesEndpoint.h"
#include "boost/algorithm/string.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

std::string toResponse(const Device& device) {
  using namespace rapidjson;

  Document message;
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);

  message.SetObject();
  auto& alloc = message.GetAllocator();
  message.AddMember("name", Value(device.name.c_str(), alloc), alloc);

  Value specs;
  specs.SetObject();
  specs.AddMember("rank", Value(device.specs.rank), alloc);
  specs.AddMember("nSlots", Value(device.specs.nSlots), alloc);
  specs.AddMember("inputTimesliceId", Value(device.specs.inputTimesliceId), alloc);
  specs.AddMember("maxInputTimeslices", Value(device.specs.maxInputTimeslices), alloc);

  Value inputs(kArrayType);
  for(auto& input : device.specs.inputs) {
    Value inputVal;
    inputVal.SetObject();

    inputVal.AddMember("binding", Value(input.binding.c_str(), alloc), alloc);
    inputVal.AddMember("sourceChannel", Value(input.sourceChannel.c_str(), alloc), alloc);
    inputVal.AddMember("timeslice", Value(input.timeslice), alloc);

    inputVal.AddMember("dataDescriptorMatcher", Value(input.dataDescriptorMatcher), alloc);
    inputVal.AddMember("origin", Value(input.origin.c_str(), alloc), alloc);
    inputVal.AddMember("description", Value(input.description.c_str(), alloc), alloc);
    inputVal.AddMember("subSpec", Value(input.subSpec), alloc);

    inputs.PushBack(inputVal, alloc);
  }
  specs.AddMember("inputs", inputs, alloc);

  Value outputs(kArrayType);
  for(auto& output : device.specs.outputs) {
    Value outputVal;
    outputVal.SetObject();

    outputVal.AddMember("binding", Value(output.binding.c_str(), alloc), alloc);
    outputVal.AddMember("sourceChannel", Value(output.channel.c_str(), alloc), alloc);
    outputVal.AddMember("timeslice", Value(output.timeslice), alloc);
    outputVal.AddMember("maxTimeslices", Value(output.maxTimeslices), alloc);

    outputVal.AddMember("origin", Value(output.origin.c_str(), alloc), alloc);
    outputVal.AddMember("description", Value(output.description.c_str(), alloc), alloc);
    outputVal.AddMember("subSpec", Value(output.subSpec), alloc);

    outputs.PushBack(outputVal, alloc);
  }
  specs.AddMember("outputs", outputs, alloc);

  Value forwards(kArrayType);
  for(auto& forward : device.specs.forwards) {
    Value forwardVal;
    forwardVal.SetObject();

    forwardVal.AddMember("binding", Value(forward.binding.c_str(), alloc), alloc);
    forwardVal.AddMember("sourceChannel", Value(forward.channel.c_str(), alloc), alloc);
    forwardVal.AddMember("timeslice", Value(forward.timeslice), alloc);
    forwardVal.AddMember("maxTimeslices", Value(forward.maxTimeslices), alloc);

    forwardVal.AddMember("dataDescriptorMatcher", Value(forward.dataDescriptorMatcher), alloc);
    forwardVal.AddMember("origin", Value(forward.origin.c_str(), alloc), alloc);
    forwardVal.AddMember("description", Value(forward.description.c_str(), alloc), alloc);
    forwardVal.AddMember("subSpec", Value(forward.subSpec), alloc);

    forwards.PushBack(forwardVal, alloc);
  }
  specs.AddMember("forwards", forwards, alloc);

  message.AddMember("specs", specs, alloc);

  message.Accept(writer);
  return std::string{buffer.GetString(), buffer.GetSize()};
}

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

void DevicesEndpoint::getDevices(const httplib::Request& input, httplib::Response& output) {
  std::string runId = input.get_header_value("runId");

  std::vector<std::string> devicesJSON{};
  auto deviceList = devicesService.getDevices(runId);
  std::transform(deviceList.begin(), deviceList.end(), std::back_inserter(devicesJSON), toResponse);

  std::string response = "{\"devices\":[" + boost::join(devicesJSON, ",") + "]}";
  output.set_content(response, "application/json");
}
