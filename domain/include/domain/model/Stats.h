#ifndef DIPROXY_STATS_H
#define DIPROXY_STATS_H

#include <string>
#include <vector>
#include <tuple>

using Timestamp = uint64_t;
using MsgCount = uint64_t;
using MsgSize = uint64_t;

struct PropertyStats {
  PropertyStats(): min(std::numeric_limits<uint64_t>::max()), avg(0), max(std::numeric_limits<uint64_t>::min()) {}
  ~PropertyStats() = default;

  uint64_t min;
  double avg;
  uint64_t max;
};

struct PropertyRange {
  PropertyRange(): min(std::numeric_limits<uint64_t>::max()), max(std::numeric_limits<uint64_t>::min()) {}
  ~PropertyRange() = default;

  uint64_t min;
  uint64_t max;
};

struct Stats {
  std::vector<std::tuple<Timestamp, MsgCount, MsgSize>> data{};
  uint64_t totalMessages{};
  uint64_t totalData{};
  PropertyStats sizeStats;
  PropertyStats durationStats;
  PropertyRange startTimeRange;
  PropertyRange creationTimeRange;
};

#endif //DIPROXY_STATS_H
