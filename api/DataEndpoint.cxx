#include "api/DataEndpoint.h"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

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

  if(message.payloadSerialization == "ARROW" || message.payloadSerialization == "ROOT") {
    Value payloadValue;
    payloadValue.SetObject();

    Document payloadDocument;
    payloadDocument.Parse(message.payload.c_str(), message.payload.size());
    for(auto it = payloadDocument.MemberBegin(); it != payloadDocument.MemberEnd(); it++) {
      Value name;
      name.CopyFrom(it->name, alloc);
      Value val;
      val.CopyFrom(it->value, alloc);

      payloadValue.AddMember(name, val, alloc);
    }

    doc.AddMember("payload", payloadValue, alloc);
  } else {
    doc.AddMember("payload", Value(message.payload.c_str(), alloc), alloc);
  }

  if(message.payloadEndianness.has_value())
    doc.AddMember("payloadEndianness", Value(message.payloadEndianness.value().c_str(), alloc), alloc);

  if(message.taskHash.has_value())
    doc.AddMember("taskHash", Value(message.taskHash.value()), alloc);

  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc.Accept(writer);
  return std::string{buffer.GetString(), buffer.GetSize()};
}

void DataEndpoint::getMessage(const httplib::Request& input, httplib::Response& output) {
  auto msgId = input.get_header_value("id");

  auto message = messageService.getMessage(msgId);
  output.set_content(toJson(message), "application/json");
}

void DataEndpoint::getMessages(const httplib::Request &input, httplib::Response &output) {
  auto count = std::stoi(input.get_header_value("count"));
  auto runId = input.get_header_value("runId");

  std::vector<std::string> messagesJson{};
  auto messages = messageService.getOldestMessages(runId, count);
  std::transform(messages.begin(), messages.end(), std::back_inserter(messagesJson), [](const Message& message) -> std::string{
    return toJson(message);
  });

  std::string response = "{\"messages\":[" + boost::join(messagesJson, ",") + "]}";
  output.set_content(response, "application/json");
}

void DataEndpoint::newerMessages(const httplib::Request& input, httplib::Response& output) {
  std::vector<std::string> devicesNames{};
  std::string devicesString = input.get_header_value("devices");
  boost::split(devicesNames, devicesString, boost::is_any_of(","));
  auto time = boost::lexical_cast<uint64_t>(input.get_header_value("time"));
  auto runId = input.get_header_value("runId");

  int count = std::numeric_limits<int>::max();
  if(input.has_header("count"))
    count = std::stoi(input.get_header_value("count"));

  std::vector<std::string> idsJson{};
  auto ids = messageService.newerMessages(runId, time, devicesNames, count);
  std::transform(ids.begin(), ids.end(), std::back_inserter(idsJson), [](const std::string& id) {
    return "\"" + id + "\"";
  });

  std::string response = "{\"messages\":[" + boost::join(idsJson, ",") + "]}";
  output.set_content(response, "application/json");
}
