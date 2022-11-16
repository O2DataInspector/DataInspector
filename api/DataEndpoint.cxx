#include "api/DataEndpoint.h"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"

Message DataEndpoint::getMessage(const httplib::Request& input, httplib::Response& output) {
  auto msgId = input.get_header_value("id");

  return messageService.getMessage(msgId);
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

#define STATS_STRING_PARAM(name, statsRequest) if(input.has_header(#name)) statsRequest.name = input.get_header_value(#name);
#define STATS_NUMBER_PARAM(name, type, statsRequest) if(input.has_header(#name)) statsRequest.name = boost::lexical_cast<type>(input.get_header_value(#name));
#define STRINGIFY(x) #x
#define STATS_REQUEST_RANGE_PARAM(name, type, statsRequest) {Range<type> range; if(input.has_header(STRINGIFY(min##name))) range.begin = boost::lexical_cast<type>(input.get_header_value(STRINGIFY(min##name))); if(input.has_header(STRINGIFY(max##name))) range.end = boost::lexical_cast<type>(input.get_header_value(STRINGIFY(max##name))); if(range.begin.has_value() || range.end.has_value()) statsRequest.name##Range = range;}

Stats DataEndpoint::getStats(const httplib::Request& input, httplib::Response& output) {
  StatsRequest request;
  STATS_STRING_PARAM(device, request)
  STATS_STRING_PARAM(messageOrigin, request)
  STATS_STRING_PARAM(description, request)
  STATS_NUMBER_PARAM(subSpecification, uint32_t, request)
  STATS_NUMBER_PARAM(firstTForbit, uint32_t, request)
  STATS_NUMBER_PARAM(tfCounter, uint32_t, request)
  STATS_NUMBER_PARAM(runNumber, uint32_t, request)
  STATS_STRING_PARAM(taskHash, request);
  STATS_STRING_PARAM(payloadSerialization, request)
  STATS_NUMBER_PARAM(payloadParts, uint32_t, request)
  STATS_NUMBER_PARAM(payloadSplitIndex, uint32_t, request)
  STATS_REQUEST_RANGE_PARAM(StartTime, uint64_t, request)
  STATS_REQUEST_RANGE_PARAM(CreationTime, uint64_t, request)
  STATS_REQUEST_RANGE_PARAM(Duration, uint64_t, request)
  STATS_REQUEST_RANGE_PARAM(PayloadSize, uint64_t, request)
  STATS_NUMBER_PARAM(count, uint64_t, request)

  return messageService.search(request);
}
