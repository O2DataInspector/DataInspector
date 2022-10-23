#include "api/DataEndpoint.h"
#include "boost/algorithm/string.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

std::string toJson(const Message& message) {
  using namespace rapidjson;

  Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("id", Value(message.id.c_str(), alloc), alloc);
  doc.AddMember("sender", Value(message.sender.c_str(), alloc), alloc);
  doc.AddMember("startTime", Value(message.startTime), alloc);
  doc.AddMember("duration", Value(message.duration), alloc);
  doc.AddMember("creationTimer", Value(message.creationTimer), alloc);
  doc.AddMember("origin", Value(message.origin.c_str(), alloc), alloc);
  doc.AddMember("description", Value(message.description.c_str(), alloc), alloc);
  doc.AddMember("subSpecification", Value(message.subSpecification), alloc);
  doc.AddMember("firstTForbit", Value(message.firstTForbit), alloc);
  doc.AddMember("tfCounter", Value(message.tfCounter), alloc);
  doc.AddMember("runNumber", Value(message.runNumber), alloc);
  doc.AddMember("payloadSize", Value(message.payloadSize), alloc);
  doc.AddMember("splitPayloadParts", Value(message.splitPayloadParts), alloc);
  doc.AddMember("payloadSerialization", Value(message.payloadSerialization.c_str(), alloc), alloc);
  doc.AddMember("payloadSplitIndex", Value(message.payloadSplitIndex), alloc);

  if(message.payload.has_value())
    doc.AddMember("payload", Value(message.payload.value().c_str(), alloc), alloc);

  if(message.taskHash.has_value())
    doc.AddMember("taskHash", Value(message.taskHash.value()), alloc);

  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc.Accept(writer);
  return std::string{buffer.GetString(), buffer.GetSize()};
}

void DataEndpoint::getMessages(const httplib::Request &input, httplib::Response &output) {
  auto count = std::stoi(input.get_param_value("count"));
  auto runId = input.get_param_value("runId");

  std::vector<std::string> messagesJson{};
  auto messages = messageService.getOldestMessages(runId, count);
  std::transform(messages.begin(), messages.end(), std::back_inserter(messagesJson), [](const Message& message) -> std::string{
    return toJson(message);
  });

  std::string response = "{\"messages\":[" + boost::join(messagesJson, ",") + "]}";
  output.set_content(response, "application/json");
}