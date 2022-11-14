#ifndef DIPROXY_STATSREQUEST_H
#define DIPROXY_STATSREQUEST_H

#include <string>
#include <optional>

template <typename T>
struct Range {
  std::optional<T> begin;
  std::optional<T> end;
};

struct StatsRequest {
  std::string device;
  std::string origin;
  std::string description;
  uint32_t subSpecification;
  uint32_t firstTForbit;
  uint32_t tfCounter;
  uint32_t runNumber;
  std::string taskHash;
  std::string payloadSerialiation;
  uint32_t payloadParts;
  uint32_t payloadSplitIndex;
  std::optional<Range<uint64_t>> startTimeRange;
  std::optional<Range<uint64_t>> creationTimerRange;
  std::optional<Range<uint64_t>> durationRange;
  std::optional<Range<uint64_t>> payloadSizeRange;
  uint64_t count;
};

#endif //DIPROXY_STATSREQUEST_H
