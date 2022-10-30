#include "api/DataEndpoint.h"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include "serialization/RootSerialization.h"
#include "serialization/ArrowSerialization.h"
#include "utils/Base64.h"

#include "arrow/buffer.h"
#include <arrow/builder.h>
#include <arrow/memory_pool.h>
#include <arrow/record_batch.h>
#include <arrow/table.h>
#include <arrow/type_traits.h>
#include <arrow/status.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/reader.h>

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
  doc.AddMember("binPayload", Value(message.payload.c_str(), alloc), alloc);

  if(message.payloadSerialization == "ARROW") {
    auto binData = Base64::decode(message.payload);
    auto buffer = std::make_shared<arrow::Buffer>((uint8_t*) binData.data(), binData.size());

    auto bufferReader = std::make_shared<arrow::io::BufferReader>(buffer);

    auto readerResult = arrow::ipc::RecordBatchStreamReader::Open(bufferReader);
    std::vector<std::shared_ptr<arrow::RecordBatch>> batches;
    auto batchReader = readerResult.ValueOrDie();
    while (true) {
      std::shared_ptr<arrow::RecordBatch> batch;
      auto next = batchReader->ReadNext(&batch);
      if (batch == nullptr) {
        break;
      }
      batches.push_back(batch);
    }

    auto tableResult = arrow::Table::FromRecordBatches(batches);

    Value jsonValue;
    auto json = ArrowSerialization::toJson(tableResult.ValueOrDie());
    jsonValue.CopyFrom(json, alloc);

    doc.AddMember("payload", jsonValue, alloc);
  } else if(message.payloadSerialization == "ROOT") {
    auto binData = Base64::decode(message.payload);
    auto obj = RootSerialization::deserialize((uint8_t*) binData.data(), binData.size());

    TString json = TBufferJSON::ToJSON(obj.get());
    Value payloadValue;
    payloadValue.SetObject();

    Document payloadDocument;
    payloadDocument.Parse(json.Data());
    for(auto it = payloadDocument.MemberBegin(); it != payloadDocument.MemberEnd(); it++) {
      Value name;
      name.CopyFrom(it->name, alloc);
      Value val;
      val.CopyFrom(it->value, alloc);

      payloadValue.AddMember(name, val, alloc);
    }

    doc.AddMember("payload", payloadValue, alloc);
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
  auto messageId = input.get_header_value("id");
  auto runId = input.get_header_value("runId");

  int count = std::numeric_limits<int>::max();
  if(input.has_header("count"))
    count = std::stoi(input.get_header_value("count"));

  std::vector<std::string> idsJson{};
  auto ids = messageService.newerMessages(runId, messageId, devicesNames, count);
  std::transform(ids.begin(), ids.end(), std::back_inserter(idsJson), [](const std::string& id) {
    return "\"" + id + "\"";
  });

  std::string response = "{\"messages\":[" + boost::join(idsJson, ",") + "]}";
  output.set_content(response, "application/json");
}
