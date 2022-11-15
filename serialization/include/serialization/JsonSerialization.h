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
#include "domain/DIMessages.h"

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

template <>
rapidjson::Document toJson<Stats>(const Stats& stats) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("totalMessages", rapidjson::Value(stats.totalMessages), alloc);
  doc.AddMember("totalData", rapidjson::Value(stats.totalData), alloc);

  rapidjson::Value timestampsValue;
  timestampsValue.SetArray();
  rapidjson::Value countsValue;
  countsValue.SetArray();
  rapidjson::Value sizesValue;
  sizesValue.SetArray();
  for(auto& entry : stats.data) {
    timestampsValue.PushBack(rapidjson::Value(std::get<0>(entry)), alloc);
    countsValue.PushBack(rapidjson::Value(std::get<1>(entry)), alloc);
    sizesValue.PushBack(rapidjson::Value(std::get<2>(entry)), alloc);
  }
  doc.AddMember("x", timestampsValue, alloc);
  doc.AddMember("yNumbers", countsValue, alloc);
  doc.AddMember("yData", sizesValue, alloc);

  doc.AddMember("minMsgSize", rapidjson::Value(stats.sizeStats.min), alloc);
  doc.AddMember("avgMsgSize", rapidjson::Value(stats.sizeStats.avg), alloc);
  doc.AddMember("maxMsgSize", rapidjson::Value(stats.sizeStats.max), alloc);

  doc.AddMember("minDuration", rapidjson::Value(stats.durationStats.min), alloc);
  doc.AddMember("avgDuration", rapidjson::Value(stats.durationStats.avg), alloc);
  doc.AddMember("maxDuration", rapidjson::Value(stats.durationStats.max), alloc);

  doc.AddMember("minStartTime", rapidjson::Value(stats.startTimeRange.min), alloc);
  doc.AddMember("maxStartTime", rapidjson::Value(stats.startTimeRange.max), alloc);

  doc.AddMember("minCreationTime", rapidjson::Value(stats.creationTimeRange.min), alloc);
  doc.AddMember("maxCreationTime", rapidjson::Value(stats.creationTimeRange.max), alloc);

  return doc;
}

template <>
rapidjson::Document toJson<Response::AnalysisBuildStatus>(const Response::AnalysisBuildStatus& buildStatus) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("buildStatus", rapidjson::Value(buildStatus.status.c_str(), alloc), alloc);

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& log : buildStatus.logs) {
    listValue.PushBack(rapidjson::Value(log.c_str(), alloc), alloc);
  }
  doc.AddMember("logs", listValue, alloc);

  return doc;
}

template <>
rapidjson::Document toJson<Response::AnalysisId>(const Response::AnalysisId& analysisId) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("analysisId", rapidjson::Value(analysisId.analysisId.c_str(), alloc), alloc);
  return doc;
}

template <>
rapidjson::Document toJson<Response::AnalysisList::Analysis>(const Response::AnalysisList::Analysis& analysis) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("buildStatus", rapidjson::Value(analysis.buildStatus.c_str(), alloc), alloc);
  doc.AddMember("name", rapidjson::Value(analysis.name.c_str(), alloc), alloc);
  doc.AddMember("id", rapidjson::Value(analysis.id.c_str(), alloc), alloc);
  doc.AddMember("url", rapidjson::Value(analysis.url.c_str(), alloc), alloc);
  doc.AddMember("branch", rapidjson::Value(analysis.branch.c_str(), alloc), alloc);

  return doc;
}

template <>
rapidjson::Document toJson<Response::AnalysisList>(const Response::AnalysisList& analysisList) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  std::vector<rapidjson::Document> objDocs{};
  std::transform(analysisList.analyses.begin(), analysisList.analyses.end(), std::back_inserter(objDocs), [](const Response::AnalysisList::Analysis& analysis) {
    return toJson(analysis);
  });

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& objDoc : objDocs) {
    rapidjson::Value objValue;
    objValue.CopyFrom(objDoc, alloc);
    listValue.PushBack(objValue, alloc);
  }

  doc.AddMember("analyses", listValue, alloc);
  return doc;
}

template <>
rapidjson::Document toJson<Response::WorkflowList>(const Response::WorkflowList& workflowList) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& workflow : workflowList.workflows) {
    listValue.PushBack(rapidjson::Value(workflow.c_str(), alloc), alloc);
  }

  doc.AddMember("workflows", listValue, alloc);
  return doc;
}

template <>
rapidjson::Document toJson<Response::RunId>(const Response::RunId& runId) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("runId", rapidjson::Value(runId.runId.c_str(), alloc), alloc);
  return doc;
}

template <>
rapidjson::Document toJson<Response::RunsList::Analysis>(const Response::RunsList::Analysis& analysis) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("name", rapidjson::Value(analysis.name.c_str(), alloc), alloc);
  doc.AddMember("id", rapidjson::Value(analysis.id.c_str(), alloc), alloc);
  doc.AddMember("url", rapidjson::Value(analysis.url.c_str(), alloc), alloc);
  doc.AddMember("branch", rapidjson::Value(analysis.branch.c_str(), alloc), alloc);

  return doc;
}

template <>
rapidjson::Document toJson<Response::RunsList::Run>(const Response::RunsList::Run& run) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  doc.AddMember("config", rapidjson::Value(run.config.c_str(), alloc), alloc);
  doc.AddMember("id", rapidjson::Value(run.id.c_str(), alloc), alloc);
  doc.AddMember("workflow", rapidjson::Value(run.workflow.c_str(), alloc), alloc);

  rapidjson::Value analysisValue;
  analysisValue.CopyFrom(toJson(run.analysis), alloc);
  doc.AddMember("analysis", analysisValue, alloc);

  return doc;
}

template <>
rapidjson::Document toJson<Response::RunsList>(const Response::RunsList& runsList) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  std::vector<rapidjson::Document> objDocs{};
  std::transform(runsList.runs.begin(), runsList.runs.end(), std::back_inserter(objDocs), [](const Response::RunsList::Run& run) {
    return toJson(run);
  });

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& objDoc : objDocs) {
    rapidjson::Value objValue;
    objValue.CopyFrom(objDoc, alloc);
    listValue.PushBack(objValue, alloc);
  }

  doc.AddMember("runs", listValue, alloc);
  return doc;
}

template <>
rapidjson::Document toJson<Response::DatasetList>(const Response::DatasetList& datasetList) {
  rapidjson::Document doc;
  doc.SetObject();
  auto& alloc = doc.GetAllocator();

  rapidjson::Value listValue;
  listValue.SetArray();
  for(auto& dataset : datasetList.datasets) {
    listValue.PushBack(rapidjson::Value(dataset.c_str(), alloc), alloc);
  }

  doc.AddMember("datasets", listValue, alloc);
  return doc;
}

#endif //DIPROXY_JSONSERIALIZATION_H
