#include "api/DataEndpoint.h"
#include "boost/algorithm/string.hpp"

std::string toJson(const Message& message) {
  return message.raw;
}

void DataEndpoint::getMessages(const httplib::Request &input, httplib::Response &output) {
  auto count = std::stoi(input.get_header_value("count"));
  auto runId = input.get_header_value("run-id");

  std::vector<std::string> messagesJson{};
  auto messages = messageService.getOldestMessages(runId, count);
  std::transform(messages.begin(), messages.end(), std::back_inserter(messagesJson), [](const Message& message) -> std::string{
    return toJson(message);
  });

  std::string response = "{\"messages\":[" + boost::join(messagesJson, ",") + "]}";
  output.set_content(response, "application/json");
}