#include "domain/MessageService.h"
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <iostream>

Message MessageService::getMessage(const std::string& id) {
  return messageRepository.getMessage(id);
}

std::vector<Message> MessageService::newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) {
  return messageRepository.newerMessages(runId, messageId, devices, count);
}

Stats MessageService::search(const StatsRequest& request) {
  auto messages = messageRepository.search(request);

  std::unordered_map<uint64_t, std::vector<Message>> msgsByTimestamp;
  std::vector<uint64_t> orderedTimestamps;
  for(auto& msg : messages) {
    if(msgsByTimestamp.count(msg.startTime) == 0) {
      msgsByTimestamp[msg.startTime] = std::vector<Message>{};
      orderedTimestamps.push_back(msg.startTime);
    }
    msgsByTimestamp[msg.startTime].push_back(msg);
  }
  std::sort(orderedTimestamps.begin(), orderedTimestamps.end());

  Stats stats;
  for(auto& timestamp : orderedTimestamps) {
    auto& msgs = msgsByTimestamp[timestamp];
    auto totalSize = std::transform_reduce(msgs.begin(), msgs.end(), (uint64_t) 0, std::plus<uint64_t>{}, [](const Message& msg) {return msg.payloadSize;});
    stats.data.emplace_back(timestamp, msgs.size(), totalSize);

    for(auto& msg : msgs) {
      if(msg.payloadSize > stats.sizeStats.max)
        stats.sizeStats.max = msg.payloadSize;
      if(msg.payloadSize < stats.sizeStats.min)
        stats.sizeStats.min = msg.payloadSize;

      if(msg.duration > stats.durationStats.max)
        stats.durationStats.max = msg.duration;
      if(msg.payloadSize < stats.durationStats.min)
        stats.durationStats.min = msg.duration;

      if(msg.startTime > stats.startTimeRange.max)
        stats.startTimeRange.max = msg.startTime;
      if(msg.startTime < stats.startTimeRange.min)
        stats.startTimeRange.min = msg.startTime;

      if(msg.creationTimer > stats.creationTimeRange.max)
        stats.creationTimeRange.max = msg.creationTimer;
      if(msg.creationTimer < stats.creationTimeRange.min)
        stats.creationTimeRange.min = msg.creationTimer;
    }
  }

  stats.totalMessages = messages.size();
  stats.totalData = std::transform_reduce(messages.begin(), messages.end(), (uint64_t) 0, std::plus<uint64_t>{}, [](const Message& msg) {return msg.payloadSize;});
  stats.sizeStats.avg = stats.totalMessages > 0 ? (stats.totalData / stats.totalMessages) : 0;
  stats.durationStats.avg = stats.totalMessages > 0 ? (std::transform_reduce(messages.begin(), messages.end(), (uint64_t) 0, std::plus<uint64_t>{}, [](const Message& msg) {return msg.duration;}) / stats.totalMessages) : 0;

  return stats;
}
