#ifndef DIPROXY_JSONSERIALIZATION_H
#define DIPROXY_JSONSERIALIZATION_H

#include "rapidjson/document.h"
#include "domain/model/Message.h"
#include "utils/Base64.h"
#include "ArrowSerialization.h"
#include "RootSerialization.h"
#include "api/response/DeviceList.h"
#include "api/response/MessageList.h"
#include "api/response/MessageHeaderList.h"

template <typename T>
rapidjson::Document toJson(const T& t);

template <>
rapidjson::Document toJson<Message>(const Message& message) {
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
    auto table = ArrowSerialization::toTable((uint8_t*) binData.c_str(), binData.size());

    Value jsonValue;
    auto json = ArrowSerialization::toJson(table);
    jsonValue.CopyFrom(json, alloc);

    doc.AddMember("payload", jsonValue, alloc);
  } else if(message.payloadSerialization == "ROOT") {
    auto binData = Base64::decode(message.payload);
    auto obj = RootSerialization::toObject((uint8_t*) binData.data(), binData.size());
    auto payloadDocument = RootSerialization::toJson(obj.get());

    Value payloadValue;
    payloadValue.SetObject();
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

  return doc;
}

template <>
rapidjson::Document toJson<Response::MessageList>(const Response::MessageList& messageList) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  std::vector<rapidjson::Document> objDocs{};
  std::transform(messageList.messages.begin(), messageList.messages.end(), std::back_inserter(objDocs), [](const Message& message) {
    return toJson(message);
  });

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& objDoc : objDocs) {
    rapidjson::Value objValue;
    objValue.CopyFrom(objDoc, alloc);
    listValue.PushBack(objValue, alloc);
  }

  doc.AddMember("messages", listValue, alloc);
  return doc;
}

template <>
rapidjson::Document toJson<Device>(const Device& device) {
  using namespace rapidjson;
  Document message;

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

    if(input.origin.has_value())
      inputVal.AddMember("origin", Value(input.origin.value().c_str(), alloc), alloc);
    if(input.description.has_value())
      inputVal.AddMember("description", Value(input.description.value().c_str(), alloc), alloc);
    if(input.subSpec.has_value())
      inputVal.AddMember("subSpec", Value(input.subSpec.value()), alloc);

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
    if(output.subSpec.has_value())
      outputVal.AddMember("subSpec", Value(output.subSpec.value()), alloc);
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

    if(forward.origin.has_value())
      forwardVal.AddMember("origin", Value(forward.origin.value().c_str(), alloc), alloc);
    if(forward.description.has_value())
      forwardVal.AddMember("description", Value(forward.description.value().c_str(), alloc), alloc);
    if(forward.subSpec.has_value())
      forwardVal.AddMember("subSpec", Value(forward.subSpec.value()), alloc);

    forwards.PushBack(forwardVal, alloc);
  }
  specs.AddMember("forwards", forwards, alloc);

  message.AddMember("specs", specs, alloc);

  return message;
}

template <>
rapidjson::Document toJson<Response::DeviceList>(const Response::DeviceList& deviceList) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  std::vector<rapidjson::Document> objDocs{};
  std::transform(deviceList.devices.begin(), deviceList.devices.end(), std::back_inserter(objDocs), [](const Device& device) {
    return toJson(device);
  });

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& objDoc : objDocs) {
    rapidjson::Value objValue;
    objValue.CopyFrom(objDoc, alloc);
    listValue.PushBack(objValue, alloc);
  }

  doc.AddMember("devices", listValue, alloc);
  return doc;
}

template <>
rapidjson::Document toJson<Response::MessageHeader>(const Response::MessageHeader& header) {
  using namespace rapidjson;

  Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("id", Value(header.id.c_str(), alloc), alloc);
  doc.AddMember("device", Value(header.device.c_str(), alloc), alloc);
  doc.AddMember("startTime", Value(header.startTime), alloc);
  doc.AddMember("duration", Value(header.duration), alloc);
  doc.AddMember("creationTimer", Value(header.creationTimer), alloc);
  doc.AddMember("origin", Value(header.origin.c_str(), alloc), alloc);
  doc.AddMember("description", Value(header.description.c_str(), alloc), alloc);
  doc.AddMember("payloadSize", Value(header.payloadSize), alloc);
  doc.AddMember("payloadSerialization", Value(header.payloadSerialization.c_str(), alloc), alloc);

  return doc;
}

template <>
rapidjson::Document toJson<Response::MessageHeaderList>(const Response::MessageHeaderList& headerList) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  std::vector<rapidjson::Document> objDocs{};
  std::transform(headerList.messages.begin(), headerList.messages.end(), std::back_inserter(objDocs), [](const Response::MessageHeader& header) {
    return toJson(header);
  });

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& objDoc : objDocs) {
    rapidjson::Value objValue;
    objValue.CopyFrom(objDoc, alloc);
    listValue.PushBack(objValue, alloc);
  }

  doc.AddMember("messages", listValue, alloc);
  return doc;
}


#endif //DIPROXY_JSONSERIALIZATION_H
