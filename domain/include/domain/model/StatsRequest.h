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
  std::string runId;
  std::optional<std::string> device;
  std::optional<std::string> messageOrigin;
  std::optional<std::string> description;
  std::optional<uint32_t> subSpecification;
  std::optional<uint32_t> firstTForbit;
  std::optional<uint32_t> tfCounter;
  std::optional<uint32_t> runNumber;
  std::optional<std::string> taskHash;
  std::optional<std::string> payloadSerialization;
  std::optional<uint32_t> payloadParts;
  std::optional<uint32_t> payloadSplitIndex;
  std::optional<Range<uint64_t>> StartTimeRange;
  std::optional<Range<uint64_t>> CreationTimeRange;
  std::optional<Range<uint64_t>> DurationRange;
  std::optional<Range<uint64_t>> PayloadSizeRange;
  std::optional<uint64_t> count;
};

#endif //DIPROXY_STATSREQUEST_H
