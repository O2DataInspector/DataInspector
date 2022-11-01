#include "api/DataEndpoint.h"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

Message DataEndpoint::getMessage(const httplib::Request& input, httplib::Response& output) {
  auto msgId = input.get_header_value("id");

  return messageService.getMessage(msgId);
}

Response::MessageList DataEndpoint::getMessages(const httplib::Request &input, httplib::Response &output) {
  auto count = std::stoi(input.get_header_value("count"));
  auto runId = input.get_header_value("runId");

  return {messageService.getOldestMessages(runId, count)};
}

Response::MessageHeaderList DataEndpoint::newerMessages(const httplib::Request& input, httplib::Response& output) {
  std::vector<std::string> devicesNames{};
  std::string devicesString = input.get_header_value("devices");
  boost::split(devicesNames, devicesString, boost::is_any_of(","));
  auto messageId = input.get_header_value("id");
  auto runId = input.get_header_value("runId");

  int count = std::numeric_limits<int>::max();
  if(input.has_header("count"))
    count = std::stoi(input.get_header_value("count"));

  std::vector<Response::MessageHeader> headers{};
  auto messages = messageService.newerMessages(runId, messageId, devicesNames, count);
  std::transform(messages.begin(), messages.end(), std::back_inserter(headers), [](const Message& message) {
    return Response::MessageHeader{
      .id = message.id,
      .device = message.sender,
      .startTime = message.startTime,
      .duration = message.duration,
      .creationTimer = message.creationTimer,
      .origin = message.origin,
      .description = message.description,
      .payloadSize = message.payloadSize,
      .payloadSerialization = message.payloadSerialization
    };
  });

  return Response::MessageHeaderList{headers};
}
